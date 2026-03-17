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

// Status codes mapping
#include "../interfaces/iDataSource.hpp"

tflDataSource::tflDataSource() : id(0), maxServicesRead(false), callback(nullptr), messagesData(4) {
    stationData = std::unique_ptr<TflStation>(new (std::nothrow) TflStation());
    if (stationData) memset(stationData.get(), 0, sizeof(TflStation));
    lastErrorMsg[0] = '\0';
    tflAppkey[0] = '\0';
    tubeId[0] = '\0';
    maxResults = 0; // 0 means no limit
    isTestMode = false;
}

void tflDataSource::configure(const char* naptanId, const char* apiKey, tflDataSourceCallback cb) {
    if (naptanId) strlcpy(tubeId, naptanId, sizeof(tubeId));
    if (apiKey) strlcpy(tflAppkey, apiKey, sizeof(tflAppkey));
    callback = cb;
}

int tflDataSource::updateData() {
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
        return UPD_DATA_ERROR;
    }

    memset(xStation.get(), 0, sizeof(TflStation));
    parser->setListener(this);
    httpsClient->setInsecure();
    httpsClient->setTimeout(5000);
    httpsClient->setConnectionTimeout(5000);

    if (!httpsClient->connect(apiHost, 443)) return UPD_NO_RESPONSE;

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
    if (retry >= 40) return UPD_TIMEOUT;

    String statusLine = httpsClient->readStringUntil('\n');
    if (!statusLine.startsWith(F("HTTP/")) || statusLine.indexOf(F("200 OK")) == -1) {
        httpsClient->stop();
        return UPD_HTTP_ERROR;
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
    while((httpsClient->available() || httpsClient->connected()) && !maxServicesRead) {
        while(httpsClient->available() && !maxServicesRead) {
            parser->parse(httpsClient->read());
            dataReceived++;
            if (millis() > ticker) { if (callback) callback(); ticker = millis() + 350; }
        }
        delay(10);
    }
    httpsClient->stop();

    if (isTestMode) {
        snprintf(lastErrorMsg, sizeof(lastErrorMsg), "AUTH SUCCESS %lums", (unsigned long)(millis()-perfTimer));
        return UPD_SUCCESS;
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
                parser->reset(); id = 0; maxServicesRead = false;
                while((httpsClient->available() || httpsClient->connected()) && !maxServicesRead) {
                    while(httpsClient->available() && !maxServicesRead) {
                        parser->parse(httpsClient->read());
                        dataReceived++;
                        if (millis() > ticker) { if (callback) callback(); ticker = millis() + 350; }
                    }
                    delay(10);
                }
            }
        }
    }
    httpsClient->stop();

    // Sort and Sanitize
    if (stationData) {
        std::sort(stationData->service, stationData->service + stationData->numServices, compareTimes);
        if (stationData->numServices > TFL_MAX_SERVICES) stationData->numServices = TFL_MAX_SERVICES;

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

    snprintf(lastErrorMsg, sizeof(lastErrorMsg), "SUCCESS %lums [%ld]", (unsigned long)(millis()-perfTimer), dataReceived);
    if (stationData) {
        LOG_INFO("DATA", "TfL (ID: " + String(tubeId) + "): Found " + String(stationData->numServices) + " services.");
#ifdef ENABLE_DEBUG_LOG
        LOG_DEBUG("DATA", "--- TfL Data ---");
        for (int i = 0; i < stationData->numServices; i++) {
            char debugMsg[256];
            snprintf(debugMsg, sizeof(debugMsg), "Service %d: %s - %s (Exp: %s, %d s)", 
                     i, stationData->service[i].lineName, stationData->service[i].destination, 
                     stationData->service[i].expectedTime, stationData->service[i].timeToStation);
            LOG_DEBUG("DATA", debugMsg);
        }
        LOG_DEBUG("DATA", "----------------");
#endif
    }
    return changed ? UPD_SUCCESS : UPD_NO_CHANGE;
}

/**
 * @brief Performs a lightweight connection and authentication test.
 * @param token Optional token to test (overrides stored configuration).
 * @return int Update status code.
 */
int tflDataSource::testConnection(const char* token) {
    LOG_INFO("DATA", "TfL Board: Performing lightweight Auth-only check via testConnection");
    
    // Save current state to avoid clobbering an active board's settings
    bool prevTestMode = isTestMode;
    char prevKey[50];
    strlcpy(prevKey, tflAppkey, sizeof(prevKey));
    
    // Configure for test
    isTestMode = true;
    if (token) {
        strlcpy(tflAppkey, token, sizeof(tflAppkey));
    }
    
    // Execute update (in test mode it just checks the line status)
    int result = updateData();
    
    // Restore state
    isTestMode = prevTestMode;
    strlcpy(tflAppkey, prevKey, sizeof(tflAppkey));
    
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
    else if (currentKey == "lineName") strncpy(stationData->service[id].lineName, val.c_str(), TFL_MAX_LINE-1);
    else if (currentKey == "description") {
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
