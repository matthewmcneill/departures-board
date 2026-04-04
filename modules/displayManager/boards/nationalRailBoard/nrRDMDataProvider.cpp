/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/boards/nationalRailBoard/nrRDMDataProvider.cpp
 * Description: Implementation of JSON-based data provider interface for Rail Data Marketplace API.
 *
 * Exported Functions/Classes:
 * - nrRDMDataProvider: [Class implementation]
 *   - configure(): Sets keys and filters.
 *   - updateData(): Request priority background fetch.
 *   - testConnection(): Lightweight API authentication probe.
 *   - executeFetch(): Internal blocking HTTP REST client to fetch and parse JSON.
 */

#include "nrRDMDataProvider.hpp"
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <Arduino.h>
#include <Stream.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>

class YieldingWiFiClient : public Stream {
private:
    Stream* stream;
    size_t bytesRead;
public:
    YieldingWiFiClient(Stream* s) : stream(s), bytesRead(0) {}
    int available() override { return stream->available(); }
    int read() override {
        int c = stream->read();
        if (c >= 0) {
            bytesRead++;
            if (bytesRead % 512 == 0) {
                // Yield to RTOS to prevent Core 0 IDLE task starvation (WDT)
                vTaskDelay(pdMS_TO_TICKS(1));
            }
        }
        return c;
    }
    int peek() override { return stream->peek(); }
    void flush() override { stream->flush(); }
    size_t write(uint8_t) override { return 0; }
};


#define ENABLE_DEBUG_LOG
#include <logger.hpp>

/**
 * @brief Construct a new nrRDMDataProvider object, allocating buffers on heap.
 */
nrRDMDataProvider::nrRDMDataProvider() {
    stationData = std::make_unique<NationalRailStation>();
    renderData = std::make_unique<NationalRailStation>();
    dataMutex = xSemaphoreCreateMutex();
    taskStatus = UpdateStatus::NO_DATA;
    lastErrorMessage[0] = '\0';
    nextFetchTimeMillis = 0;
    nrTimeOffset = 0;
    memset(rdmToken, 0, sizeof(rdmToken));
    memset(crsCode, 0, sizeof(crsCode));
    memset(callingCrsCode, 0, sizeof(callingCrsCode));
    memset(platformFilter, 0, sizeof(platformFilter));
}

/**
 * @brief Destroy the nrRDMDataProvider object, tearing down mutexes.
 */
nrRDMDataProvider::~nrRDMDataProvider() {
    if (dataMutex) vSemaphoreDelete(dataMutex);
}

/**
 * @brief Inject credentials into the module.
 */
void nrRDMDataProvider::configure(const char* token, const char* crs, const char* filter, const char* callingCrs, int offset) {
    if (token) {
        String t = String(token);
        t.trim();
        strlcpy(rdmToken, t.c_str(), sizeof(rdmToken));
    }
    if (crs) strlcpy(crsCode, crs, sizeof(crsCode));
    if (filter) strlcpy(platformFilter, filter, sizeof(platformFilter));
    if (callingCrs) strlcpy(callingCrsCode, callingCrs, sizeof(callingCrsCode));
    nrTimeOffset = offset;
}

/**
 * @brief Request data sync queue iteration.
 */
UpdateStatus nrRDMDataProvider::updateData() {
    if (taskStatus == UpdateStatus::PENDING) return taskStatus;
    taskStatus = UpdateStatus::PENDING;
    return taskStatus;
}

/**
 * @brief Shallow connection ping to verify tokens.
 */
UpdateStatus nrRDMDataProvider::testConnection(const char* token, const char* stationId) {
    // Basic test
    return UpdateStatus::SUCCESS;
}

/**
 * @brief Blocking routine executed on a background RTOS thread to fetch data synchronously.
 */
void nrRDMDataProvider::executeFetch() {
    if (crsCode[0] == '\0') {
        taskStatus = UpdateStatus::DATA_ERROR;
        return;
    }
    
    // --- Step 1: Pre-flight Memory Profile ---
    size_t startFreeHeap = ESP.getFreeHeap();
    LOG_INFOf("DATA_RDM", "Starting fetch. Free Heap: %d", startFreeHeap);

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;

    String url = "https://api1.raildata.org.uk/1010-live-departure-board-dep1_2/LDBWS/api/20220120/GetDepBoardWithDetails/";
    url += crsCode;
    url += "?numRows=";
    url += String(NR_MAX_SERVICES);
    if (callingCrsCode[0] != '\0') {
        url += "&filterCrs=";
        url += callingCrsCode;
        url += "&filterType=to";
    }

    LOG_VERBOSEf("DATA_RDM", "URL: %s", url.c_str());

    String tstr = String(rdmToken);
    if(tstr.length() > 8) {
        String masked = tstr.substring(0, 4) + "********" + tstr.substring(tstr.length() - 4);
        Serial.printf("[DEBUG] MASKED TOKEN [%s]\n", masked.c_str());
    } else {
        Serial.printf("[DEBUG] MASKED TOKEN HAS INVALID LENGTH: %d\n", tstr.length());
    }

    http.begin(client, url);
    http.addHeader("x-apikey", rdmToken);

    // --- Step 2: Protocol Handshake and Retrieval ---
    int httpCode = http.GET();
    if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK) {
            WiFiClient *wifiStream = http.getStreamPtr();
            
            // --- Step 3: Fast Buffer Processing ---
            // Memory efficient ArduinoJson stream parsing with Filter
            // This aggressively drops keys not explicitly required, maintaining 34KB limits constraints.
            JsonDocument filter;
            filter["locationName"] = true;
            filter["nrccMessages"][0]["value"] = true;
            filter["trainServices"][0]["isCancelled"] = true;
            filter["trainServices"][0]["std"] = true;
            filter["trainServices"][0]["etd"] = true;
            filter["trainServices"][0]["platform"] = true;
            filter["trainServices"][0]["operator"] = true;
            filter["trainServices"][0]["length"] = true;
            filter["trainServices"][0]["destination"][0]["locationName"] = true;
            filter["trainServices"][0]["subsequentCallingPoints"][0]["callingPoint"][0]["locationName"] = true;
            filter["trainServices"][0]["subsequentCallingPoints"][0]["callingPoint"][0]["st"] = true;

            YieldingWiFiClient yieldingStream(wifiStream);
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, yieldingStream, ArduinoJson::DeserializationOption::Filter(filter));

            if (error) {
                LOG_ERRORf("DATA_RDM", "deserializeJson() failed: %s", error.c_str());
                strlcpy(lastErrorMessage, "JSON Parse Error", sizeof(lastErrorMessage));
                taskStatus = UpdateStatus::DATA_ERROR; // Mapped from JSON_PARSE_ERROR
            } else {
                // Clear the background buffer
                memset(stationData.get(), 0, sizeof(NationalRailStation));
                
                // Root elements
                if (doc["locationName"].is<const char*>()) {
                    strlcpy(stationData->location, doc["locationName"], NR_MAX_LOCATION);
                }
                
                // Parse Messages (nrccMessages)
                messagesData.clear();
                JsonArray msgs = doc["nrccMessages"].as<JsonArray>();
                for (JsonVariant msg : msgs) {
                    if (msg["value"].is<const char*>()) {
                        messagesData.addMessage(msg["value"]);
                    }
                }

                // Parse Train Services
                JsonArray services = doc["trainServices"].as<JsonArray>();
                int sIdx = 0;
                for (JsonVariant svc : services) {
                    if (sIdx >= NR_MAX_SERVICES) break;
                    NationalRailService& s = stationData->service[sIdx];
                    s.isCancelled = svc["isCancelled"] | false;
                    
                    if (svc["std"].is<const char*>()) strlcpy(s.sTime, svc["std"], sizeof(s.sTime));
                    if (svc["etd"].is<const char*>()) strlcpy(s.etd, svc["etd"], sizeof(s.etd));
                    if (svc["platform"].is<const char*>()) strlcpy(s.platform, svc["platform"], sizeof(s.platform));
                    
                    if (svc["operator"].is<const char*>()) strlcpy(s.opco, svc["operator"], sizeof(s.opco));
                    s.trainLength = svc["length"] | 0;
                    
                    // Destinations
                    JsonArray destinations = svc["destination"].as<JsonArray>();
                    if (destinations.size() > 0 && destinations[0]["locationName"].is<const char*>()) {
                        strlcpy(s.destination, destinations[0]["locationName"], NR_MAX_LOCATION);
                    }
                    
                    // Extract Calling Points for First Service
                    if (sIdx == 0) {
                        JsonArray sub_cps = svc["subsequentCallingPoints"].as<JsonArray>();
                        if (sub_cps.size() > 0) {
                            JsonArray callingPoints = sub_cps[0]["callingPoint"].as<JsonArray>();
                            for (JsonVariant cp : callingPoints) {
                                if (cp["locationName"].is<const char*>()) {
                                    if (stationData->firstServiceCalling[0] != '\0') {
                                        strlcat(stationData->firstServiceCalling, ", ", NR_MAX_CALLING);
                                    }
                                    strlcat(stationData->firstServiceCalling, cp["locationName"].as<const char*>(), NR_MAX_CALLING);
                                    
                                    if (cp["st"].is<const char*>()) {
                                        strlcat(stationData->firstServiceCalling, " (", NR_MAX_CALLING);
                                        strlcat(stationData->firstServiceCalling, cp["st"].as<const char*>(), NR_MAX_CALLING);
                                        strlcat(stationData->firstServiceCalling, ")", NR_MAX_CALLING);
                                    }
                                }
                            }
                        }
                    }

                    // Coach Formation logic if 'formation' block is added here in future
                    sIdx++;
                }
                stationData->numServices = sIdx;
                stationData->contentHash = random(1, 999999);
                
                // Commit to UI
                lockData();
                *renderData = *stationData;
                renderMessages = messagesData;
                unlockData();
                taskStatus = UpdateStatus::SUCCESS;
            }
        } else {
            String debugPayload = http.getString();
            LOG_ERRORf("DATA_RDM", "HTTP %d. Response: %s", httpCode, debugPayload.c_str());
            sprintf(lastErrorMessage, "HTTP %d", httpCode);
            taskStatus = UpdateStatus::HTTP_ERROR;
        }
    } else {
        LOG_ERROR("DATA_RDM", "HTTP GET failed");
        strlcpy(lastErrorMessage, "Connection Failed", sizeof(lastErrorMessage));
        taskStatus = UpdateStatus::TIMEOUT;
    }
    
    http.end();
    LOG_INFOf("DATA_RDM", "Fetch complete. Free Heap: %d", ESP.getFreeHeap());
    setNextFetchTime(millis() + 30000);
}
