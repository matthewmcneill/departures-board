/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/boards/tflBoard/tflDataSource.cpp
 * Description: Implementation of TfL data source ported from TfLdataClient.
 *
 * Exported Functions/Classes:
 * - tflDataSource: Data client for TfL Unified API.
 *   - configure(): Sets Naptan ID and optional API key.
 *   - updateData(): Performs SSL GET request and parses JSON response.
 *   - getStation(): Accessor for parsed station metadata.
 *   - getServicesCount(): Returns current found arrivals.
 *   - getService(): Accessor for individual arrival data.
 *   - getMessagesCount(): Accessor for line disruption messages.
 */

#include "tflDataSource.hpp"
#include <WiFiClientSecure.h>
#include <logger.hpp>
#include <algorithm>
#include <memory>
#include <appContext.hpp>

// Status codes mapping
#include "../../../dataManager/iDataSource.hpp"

extern class appContext appContext;

const char* tflDataSource::serviceNumbers[TFL_MAX_FETCH] = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "20" };

tflDataSource::tflDataSource() : id(0), maxServicesRead(false), callback(nullptr), messagesData(4), renderMessages(4), nextFetchTimeMillis(0) {
    stationData = std::unique_ptr<TflStation>(new (std::nothrow) TflStation());
    renderData = std::unique_ptr<TflStation>(new (std::nothrow) TflStation());
    if (stationData) memset(stationData.get(), 0, sizeof(TflStation));
    if (renderData) memset(renderData.get(), 0, sizeof(TflStation));
    
    dataMutex = xSemaphoreCreateMutex();
    taskStatus = UPD_NO_DATA;

    lastErrorMsg[0] = '\0';
    tflAppkey[0] = '\0';
    tubeId[0] = '\0';
    lineFilter[0] = '\0';
    maxResults = 0; // 0 means no limit
    isTestMode = false;
}

void tflDataSource::configure(const char* naptanId, const char* apiKey, tflDataSourceCallback cb) {
    if (naptanId) strlcpy(tubeId, naptanId, sizeof(tubeId));
    if (apiKey) strlcpy(tflAppkey, apiKey, sizeof(tflAppkey));
    callback = cb;
}

int tflDataSource::updateData() {
    if (taskStatus == UPD_PENDING) {
        return UPD_PENDING;
    }
    
    LOG_INFO("DATA", "TfL Source: Requesting priority fetch from DataManager");
    taskStatus = UPD_PENDING;
    appContext.getDataManager().requestPriorityFetch(this);
    return UPD_PENDING;
}

uint8_t tflDataSource::getPriorityTier() {
    if (renderData && renderData->numServices == 0) return TIER_HIGH;
    return TIER_MEDIUM;
}

/**
 * @brief Internal blocking method that executes the API protocol and coordinates JSON parse.
 */
void tflDataSource::executeFetch() {
    unsigned long perfTimer = millis();
    long dataReceived = 0;
    bool bChunked = false;
    lastErrorMsg[0] = '\0';

    // Offload TflStation and Clients to heap to save stack
    std::unique_ptr<TflStation> xStation(new (std::nothrow) TflStation());
    std::unique_ptr<JsonStreamingParser> parser(new (std::nothrow) JsonStreamingParser());
    std::unique_ptr<WiFiClientSecure> httpsClient(new (std::nothrow) WiFiClientSecure());

    if (!xStation || !parser || !httpsClient) {
        LOG_ERROR("DATA", "TfL Board: Memory allocation failed!");
        taskStatus = UPD_DATA_ERROR;
        setNextFetchTime(millis() + BASELINE_MIN_INTERVAL);
        return;
    }

    memset(xStation.get(), 0, sizeof(TflStation));
    parser->setListener(this);
    httpsClient->setInsecure();
    httpsClient->setTimeout(5000);
    httpsClient->setConnectionTimeout(5000);

    if (!httpsClient->connect(apiHost, 443)) {
        taskStatus = UPD_NO_RESPONSE;
        setNextFetchTime(millis() + BASELINE_MIN_INTERVAL);
        return;
    }

    String request;
    if (isTestMode) {
        LOG_INFO("DATA", "TfL Board: Performing lightweight Auth-only check (Victoria Line Status)");
        request = "GET /Line/victoria/Status?app_key=" + String(tflAppkey);
    } else {
        request = "GET /StopPoint/" + String(tubeId) + F("/Arrivals?app_key=") + String(tflAppkey);
        if (maxResults > 0) request += "&top=" + String(maxResults);
    }
    
    request += F(" HTTP/1.0\r\nHost: ");
    request += String(apiHost);
    request += F("\r\nConnection: close\r\n\r\n");
    
    httpsClient->print(request);
    if (callback) callback();

    unsigned long ticker = millis() + 350;
    int retry = 0;
    while(!httpsClient->available() && retry < 40) { delay(200); retry++; }
    if (retry >= 40) {
        taskStatus = UPD_TIMEOUT;
        setNextFetchTime(millis() + BASELINE_MIN_INTERVAL);
        return;
    }

    String statusLine = httpsClient->readStringUntil('\n');
    if (!statusLine.startsWith(F("HTTP/")) || statusLine.indexOf(F("200 OK")) == -1) {
        httpsClient->stop();
        taskStatus = UPD_HTTP_ERROR;
        setNextFetchTime(millis() + BASELINE_MIN_INTERVAL);
        return;
    }

    while (httpsClient->connected() || httpsClient->available()) {
        String line = httpsClient->readStringUntil('\n');
        if (line == F("\r")) break;
        if (line.startsWith(F("Transfer-Encoding:")) && line.indexOf(F("chunked")) >= 0) bChunked = true;
    }

    // Save current station data to compare later
    std::unique_ptr<TflStation> oldStation(new (std::nothrow) TflStation());
    if (oldStation && stationData) {
        memcpy(oldStation.get(), stationData.get(), sizeof(TflStation));
    }

    // Temporarily point to local xStation for parsing
    if (stationData) memcpy(stationData.get(), xStation.get(), sizeof(TflStation));
    
    // Clear message pool before parsing new ones
    messagesData.clear();
    
    id = 0; maxServicesRead = false;
    int yieldCounter = 0;
    while((httpsClient->available() || httpsClient->connected()) && !maxServicesRead) {
        while(httpsClient->available() && !maxServicesRead) {
            parser->parse(httpsClient->read());
            dataReceived++;
            // --- Arcane Logic ---
            // On single-core ESP32 variants (e.g. ESP32-C3), the Wi-Fi stack and user application
            // share the exact same processor core tightly via the RTOS scheduler. By explicitly
            // yielding execution context via vTaskDelay(1) every 500 byte blocks, we guarantee
            // network hardware interrupts service without triggering Task Watchdog Timers (TWDT).
            yieldCounter++;
            if (yieldCounter % 500 == 0) vTaskDelay(1);
            if (millis() > ticker) { if (callback) callback(); ticker = millis() + 350; }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    httpsClient->stop();

    if (isTestMode) {
        snprintf(lastErrorMsg, sizeof(lastErrorMsg), "AUTH SUCCESS %lums", (unsigned long)(millis()-perfTimer));
        taskStatus = UPD_SUCCESS;
        setNextFetchTime(millis() + BASELINE_MIN_INTERVAL);
        return;
    }

    // Fetch Disruption Msgs
    if (httpsClient->connect(apiHost, 443)) {
        request = "GET /StopPoint/" + String(tubeId) + F("/Disruption?getFamily=true&flattenResponse=true&app_key=") + String(tflAppkey) + F(" HTTP/1.0\r\nHost: ") + String(apiHost) + F("\r\nConnection: close\r\n\r\n");
        httpsClient->print(request);
        retry = 0;
        while(!httpsClient->available() && retry < 40) { delay(200); retry++; }
        if (httpsClient->available()) {
            statusLine = httpsClient->readStringUntil('\n');
            if (statusLine.indexOf(F("200 OK")) != -1) {
                while (httpsClient->connected() || httpsClient->available()) { if (httpsClient->readStringUntil('\n') == F("\r")) break; }
                parser->reset(); id = 0; maxServicesRead = false; yieldCounter = 0;
                while((httpsClient->available() || httpsClient->connected()) && !maxServicesRead) {
                    while(httpsClient->available() && !maxServicesRead) {
                        parser->parse(httpsClient->read());
                        dataReceived++;
                        yieldCounter++;
                        if (yieldCounter % 500 == 0) vTaskDelay(1);
                        if (millis() > ticker) { if (callback) callback(); ticker = millis() + 350; }
                    }
                    vTaskDelay(pdMS_TO_TICKS(10));
                }
            }
        }
    }
    httpsClient->stop();

    // Sort and Sanitize
    if (stationData) {
        std::sort(stationData->service, stationData->service + stationData->numServices, compareTimes);
        if (stationData->numServices > TFL_MAX_SERVICES) stationData->numServices = TFL_MAX_SERVICES;

        // --- Step 2: Zero-Copy Position Numbering ---
        // Assign stable pointers to position strings ("1", "2", ...) to avoid
        // stack-allocated string corruption in the UI layer.
        for (int i = 0; i < stationData->numServices; i++) {
            stationData->service[i].orderNum = serviceNumbers[i % TFL_MAX_FETCH];
        }

        // Populate expectedTime strings
        for (int i=0; i<stationData->numServices; i++) {
            int m = stationData->service[i].timeToStation / 60;
            if (m == 0) strcpy(stationData->service[i].expectedTime, "Due");
            else sprintf(stationData->service[i].expectedTime, "%d mins", m);
        }
    }

    bool changed = true;
    if (oldStation && stationData) {
        changed = (memcmp(stationData.get(), oldStation.get(), sizeof(TflStation)) != 0);
    }

    if (changed && stationData) {
        stationData->boardChanged = true;
    } else if (oldStation && stationData) {
        memcpy(stationData.get(), oldStation.get(), sizeof(TflStation));
    }

    xSemaphoreTake(dataMutex, portMAX_DELAY);
    if (stationData && renderData) {
        memcpy(renderData.get(), stationData.get(), sizeof(TflStation));
    }
    renderMessages.clear();
    for (int i = 0; i < messagesData.getCount(); i++) {
        renderMessages.addMessage(messagesData.getMessage(i));
    }
    taskStatus = changed ? UPD_SUCCESS : UPD_NO_CHANGE;
    xSemaphoreGive(dataMutex);

    snprintf(lastErrorMsg, sizeof(lastErrorMsg), "SUCCESS %lums [%ld]", (unsigned long)(millis()-perfTimer), dataReceived);
    if (renderData) {
        LOG_INFO("DATA", "TfL (ID: " + String(tubeId) + "): Found " + String(renderData->numServices) + " services.");
#ifdef ENABLE_DEBUG_LOG
        UBaseType_t hwm = uxTaskGetStackHighWaterMark(NULL);
        LOG_DEBUG("DATA", "TfL Task Stack High Water Mark: " + String(hwm) + " words");
        LOG_DEBUG("DATA", "--- TfL Data ---");
        for (int i = 0; i < renderData->numServices; i++) {
            char debugMsg[256];
            snprintf(debugMsg, sizeof(debugMsg), "Service %d: %s - %s (Exp: %s, %d s)", 
                     i, renderData->service[i].lineName, renderData->service[i].destination, 
                     renderData->service[i].expectedTime, renderData->service[i].timeToStation);
            LOG_DEBUG("DATA", debugMsg);
        }
        LOG_DEBUG("DATA", "----------------");
#endif
    }
    
    uint32_t interval = BASELINE_MIN_INTERVAL;
    if (renderData && renderData->numServices > 0) interval = 45000;
    setNextFetchTime(millis() + interval);
    LOG_INFO("DATA", "TfL Source: executeFetch() finished. Next fetch in " + String(interval) + "ms.");
    return;
}

/**
 * @brief Performs a lightweight connection and authentication test.
 * @param token Optional token to test (overrides stored configuration).
 * @return int Update status code.
 */
int tflDataSource::testConnection(const char* token, const char* stationId) {
    LOG_INFO("DATA", "TfL Board: Performing lightweight Auth-only check via testConnection");
    
    // Save current state to avoid clobbering an active board's settings
    bool prevTestMode = isTestMode;
    char prevKey[50];
    char prevTubeId[13];
    strlcpy(prevKey, tflAppkey, sizeof(prevKey));
    strlcpy(prevTubeId, tubeId, sizeof(prevTubeId));
    
    // Configure for test
    isTestMode = (stationId == nullptr || strlen(stationId) == 0);
    if (token) {
        strlcpy(tflAppkey, token, sizeof(tflAppkey));
    }
    
    if (stationId && strlen(stationId) > 0) {
        strlcpy(tubeId, stationId, sizeof(tubeId));
    }
    
    // Execute update
    // Synchronously execute it for testConnection
    executeFetch();
    int result = taskStatus;
    
    // Restore state
    isTestMode = prevTestMode;
    strlcpy(tflAppkey, prevKey, sizeof(tflAppkey));
    strlcpy(tubeId, prevTubeId, sizeof(tubeId));
    
    // Convert UPD_NO_CHANGE (1) to UPD_SUCCESS (0) for test result consistency since we don't care about board state changing
    if (result == UPD_NO_CHANGE) result = UPD_SUCCESS;

    return result;
}

bool tflDataSource::pruneFromPhrase(char* input, const char* target) {
    char* pos = strstr(input, target);
    if (pos) { *pos = '\0'; return true; }
    return false;
}

void tflDataSource::replaceWord(char* input, const char* target, const char* replacement) {
    char* pos = strstr(input, target);
    while (pos) {
        size_t tLen = strlen(target);
        size_t rLen = strlen(replacement);
        memmove(pos + rLen, pos + tLen, strlen(pos + tLen) + 1);
        memcpy(pos, replacement, rLen);
        pos = strstr(pos + rLen, target);
    }
}

bool tflDataSource::compareTimes(const TflService& a, const TflService& b) {
    return a.timeToStation < b.timeToStation;
}

void tflDataSource::whitespace(char c) {}
void tflDataSource::startDocument() {}
void tflDataSource::key(String key) {
    currentKey = key;
    if (currentKey == "id" && stationData) {
        if (stationData->numServices < TFL_MAX_FETCH) id = stationData->numServices++;
        else maxServicesRead = true;
    } else if (currentKey == "description") {
        // No manual index needed for MessagePool addition in value()
    }
}
void tflDataSource::value(String val) {
    if (!stationData) return;
    if (currentKey == "destinationName") {
        if (val.endsWith(" Underground Station")) val = val.substring(0, val.length()-20);
        else if (val.endsWith(" DLR Station")) val = val.substring(0, val.length()-12);
        strncpy(stationData->service[id].destination, val.c_str(), TFL_MAX_LOCATION-1);
    } else if (currentKey == "timeToStation") stationData->service[id].timeToStation = val.toInt();
    else if (currentKey == "lineName") {
        strncpy(stationData->service[id].lineName, val.c_str(), TFL_MAX_LINE-1);
        if (strlen(lineFilter) > 0) {
            String ln = val;
            String flt = lineFilter;
            ln.toLowerCase();
            flt.toLowerCase();
            if (ln.indexOf(flt) == -1 && flt.indexOf(ln) == -1) {
                if (stationData->numServices > 0) {
                    stationData->numServices--;
                }
                return; // Early exit to skip other keys for this filtered service
            }
        }
    } else if (currentKey == "directionName" || currentKey == "platformName") {
        if (directionFilter > 0) {
            String dir = val;
            dir.toLowerCase();
            bool match = false;
            if (directionFilter == 1 && (dir.indexOf("inbound") >= 0 || dir.indexOf("southbound") >= 0 || dir.indexOf("eastbound") >= 0)) match = true;
            else if (directionFilter == 2 && (dir.indexOf("outbound") >= 0 || dir.indexOf("northbound") >= 0 || dir.indexOf("westbound") >= 0)) match = true;
            
            if (!match) {
                if (stationData->numServices > 0) {
                    stationData->numServices--;
                }
            }
        }
    } else if (currentKey == "description") {
        // Clean and add message
        String cleanVal = val;
        cleanVal.replace("\\n", "");
        messagesData.addMessage(cleanVal.c_str());
    }
}
void tflDataSource::endArray() {}
void tflDataSource::endObject() {}
void tflDataSource::endDocument() {}
void tflDataSource::startArray() {}
void tflDataSource::startObject() {}
