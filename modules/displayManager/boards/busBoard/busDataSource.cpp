/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/boards/busBoard/busDataSource.cpp
 * Description: Implementation of busDataSource.
 *
 * Exported Functions/Classes:
 * - busDataSource: [Class implementation]
 *   - configure(): Sets the ATCO code and route filters.
 *   - updateData(): Initiates JSON fetch.
 *   - executeFetch(): Internal synchronous HTTPS scraping pipeline.
 *   - testConnection(): Validates station IDs.
 *   - getStopLongName(): Fetches friendly stop name via JSON API.
 *   - cleanFilter(): Normalizes raw filter strings.
 */

#include "busDataSource.hpp"
#include <WiFiClientSecure.h>
#include <logger.hpp>
#include <appContext.hpp>


extern class appContext appContext;

const char* busDataSource::serviceNumbers[BUS_MAX_FETCH] = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "20" };

// HTML Scraper parsing states
#define PBT_START 0
#define PBT_HEADER 1
#define PBT_SERVICE 2
#define PBT_DESTINATION 3
#define PBT_SCHEDULED 4
#define PBT_EXPECTED 5

/**
 * @brief Initialize the Bus data source.
 * Allocates background and render buffers (BusStop) and creates synchronization mutexes.
 */
busDataSource::busDataSource() : callback(nullptr), messagesData(4), renderMessages(4), nextFetchTimeMillis(0) {
    stationData = std::unique_ptr<BusStop>(new (std::nothrow) BusStop());
    renderData = std::unique_ptr<BusStop>(new (std::nothrow) BusStop());
    if (stationData) memset(stationData.get(), 0, sizeof(BusStop));
    if (renderData) memset(renderData.get(), 0, sizeof(BusStop));
    
    dataMutex = xSemaphoreCreateMutex();
    taskStatus = UpdateStatus::NO_DATA;

    lastErrorMsg[0] = '\0';
    busAtco[0] = '\0';
    busFilter[0] = '\0';
    cleanBusFilter[0] = '\0';
}

/**
 * @brief Configure station ID and route filters.
 * @param atco ATCO code for the bus stop.
 * @param filter CSV list of routes to include.
 * @param cb Feedback callback for connection status.
 */
void busDataSource::configure(const char* atco, const char* filter, busDataSourceCallback cb) {
    if (atco) strlcpy(busAtco, atco, sizeof(busAtco));
    if (filter) strlcpy(busFilter, filter, sizeof(busFilter));
    callback = cb;
    cleanFilter(busFilter, cleanBusFilter, sizeof(cleanBusFilter));
}

/**
 * @brief Trigger an asynchronous data refresh from bustimes.org.
 * Marks the task as pending and requests a priority fetch from the DataManager.
 * @return UpdateStatus::PENDING.
 */
UpdateStatus busDataSource::updateData() {
    if (taskStatus == UpdateStatus::PENDING) {
        return UpdateStatus::PENDING;
    }
    
    if (taskStatus == UpdateStatus::SUCCESS) {
        taskStatus = UpdateStatus::NO_CHANGE;
        return UpdateStatus::SUCCESS;
    }
    if (taskStatus == UpdateStatus::NO_CHANGE) {
        return UpdateStatus::NO_CHANGE;
    }

    LOG_INFO("DATA", "Bus Source: Requesting priority fetch from DataManager");
    taskStatus = UpdateStatus::PENDING;
    appContext.getDataManager().requestPriorityFetch(this);
    return UpdateStatus::PENDING;
}

PriorityTier busDataSource::getPriorityTier() {
    if (renderData && renderData->numServices == 0) return PriorityTier::PRIO_HIGH;
    return PriorityTier::PRIO_MEDIUM;
}

/**
 * @brief Internal blocking method that executes the HTTPS protocol and coordinates HTML scraping.
 */
void busDataSource::executeFetch() {
    // --- Step 1: Initialize connection ---
    unsigned long perfTimer = millis();
    long dataReceived = 0;
    bool bChunked = false;
    lastErrorMsg[0] = '\0';

    // [HYBRID-MEM] Transient parser strings are cleared at the end of every fetch cycle.
    currentKey.clear();
    currentObject.clear();
    longName.clear();

    std::unique_ptr<WiFiClientSecure> httpsClient(new (std::nothrow) WiFiClientSecure());
    if (!httpsClient) {
        LOG_ERROR("DATA", "Bus Board: Memory allocation failed for client!");
        taskStatus = UpdateStatus::DATA_ERROR;
        setNextFetchTime(millis() + BASELINE_MIN_INTERVAL);
        return;
    }

    httpsClient->setInsecure(); // No cert validation for simplicity on ESP32
    httpsClient->setTimeout(5000);
    httpsClient->setConnectionTimeout(5000);
    if (stationData) stationData->boardChanged = false;

    int retryCounter = 0;
    while (!httpsClient->connect(apiHost, 443) && (retryCounter++ < 10)) {
        delay(200);
    }
    if (retryCounter >= 10) {
        strcpy(lastErrorMsg, "Connection timeout");
        LOG_WARN("DATA", lastErrorMsg);
        taskStatus = UpdateStatus::NO_RESPONSE;
        setNextFetchTime(millis() + BASELINE_MIN_INTERVAL);
        return;
    }

    // --- Step 2: Send HTTP Request ---
    char request[256];
    int len = snprintf(request, sizeof(request), "GET /stops/%s/departures HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n", busAtco, apiHost);
    
    if (len >= (int)sizeof(request)) {
        LOG_ERRORf("DATA", "Bus Source: Request URL too long! (%d bytes)", len);
        httpsClient->stop();
        taskStatus = UpdateStatus::DATA_ERROR;
        setNextFetchTime(millis() + BASELINE_MIN_INTERVAL);
        return;
    }
    
    httpsClient->print(request);
    if (callback) callback();
    
    unsigned long ticker = millis() + 800;
    retryCounter = 0;
    while(!httpsClient->available() && retryCounter++ < 40) {
        delay(200);
    }

    if (!httpsClient->available()) {
        httpsClient->stop();
        strcpy(lastErrorMsg, "Response timeout");
        LOG_WARN("DATA", lastErrorMsg);
        taskStatus = UpdateStatus::TIMEOUT;
        setNextFetchTime(millis() + BASELINE_MIN_INTERVAL);
        return;
    }

    // --- Step 3: Parse HTTP Headers ---
    String statusLine = httpsClient->readStringUntil('\n');
    if (!statusLine.startsWith(F("HTTP/")) || statusLine.indexOf(F("200 OK")) == -1) {
        httpsClient->stop();
        if (statusLine.indexOf(F("401")) > 0 || statusLine.indexOf(F("429")) > 0) {
            strcpy(lastErrorMsg, "Not Authorized");
            taskStatus = UpdateStatus::UNAUTHORISED;
            setNextFetchTime(millis() + BASELINE_MIN_INTERVAL);
            return;
        } else {
            strlcpy(lastErrorMsg, statusLine.c_str(), sizeof(lastErrorMsg));
            taskStatus = UpdateStatus::HTTP_ERROR;
            setNextFetchTime(millis() + BASELINE_MIN_INTERVAL);
            return;
        }
    }

    while (httpsClient->connected() || httpsClient->available()) {
        String line = httpsClient->readStringUntil('\n');
        if (line == F("\r")) break;
        if (line.startsWith(F("Transfer-Encoding:")) && line.indexOf(F("chunked")) >= 0) bChunked = true;
    }

    // --- Step 4: Scraping Data from HTML Body ---
    unsigned long dataSendTimeout = millis() + 10000UL;
    id = 0;
    maxServicesRead = false;
    
    // Create a local copy to compare changes
    std::unique_ptr<BusStop> xBusStop(new (std::nothrow) BusStop());
    if (!xBusStop) {
        LOG_ERROR("DATA", "Bus Board: Memory allocation failed for xBusStop!");
        taskStatus = UpdateStatus::DATA_ERROR;
        setNextFetchTime(millis() + BASELINE_MIN_INTERVAL);
        return;
    }

    xBusStop->numServices = 0;
    for (int i=0; i<BUS_MAX_SERVICES; i++) {
        strcpy(xBusStop->service[i].destination, "Check front of bus");
        strcpy(xBusStop->service[i].sTime, "");
        strcpy(xBusStop->service[i].expectedTime, "");
    }

    int parseStep = PBT_START;
    int dataColumns = 0;
    bool serviceData = false;
    String serviceId;
    int yieldCounter = 0;

    while((httpsClient->available() || httpsClient->connected()) && (millis() < dataSendTimeout) && (!maxServicesRead)) {
        while(httpsClient->available() && !maxServicesRead) {
            // [HYBRID-MEM] Using String for line matching to handle unpredictable HTML row formats.
            String line = httpsClient->readStringUntil('\n');
            dataReceived += line.length() + 1;
            
            // --- Arcane Logic ---
            // On single-core ESP32 variants (e.g. ESP32-C3), the Wi-Fi stack and user application
            // share the exact same processor core tightly via the RTOS scheduler. By explicitly
            // yielding execution context via vTaskDelay(1) every 50 iterations, we guarantee
            // network hardware interrupts service without triggering Task Watchdog Timers (TWDT).
            yieldCounter++;
            if (yieldCounter % 50 == 0) vTaskDelay(1); // Yield when scraping large HTML tables
            
            line.trim();
            if (line.length()) {
                if (line.indexOf("</body>") >= 0) {
                    maxServicesRead = true;
                } else {
                    // Primitive state machine for HTML table parsing
                    switch (parseStep) {
                        case PBT_START:
                            if (line.indexOf("<tr>") >= 0) parseStep = PBT_HEADER;
                            break;
                        case PBT_HEADER:
                            if (line.indexOf("</tr>") >= 0) {
                                parseStep = PBT_SERVICE;
                                serviceData = false;
                            }
                            else if (line.substring(0,1) == "<") dataColumns++;
                            break;
                        case PBT_SERVICE:
                            if (line.indexOf("</table>") >= 0) {
                                dataColumns = 0;
                                parseStep = PBT_START;
                            }
                            else if (line.indexOf("</td>") >= 0) parseStep = PBT_DESTINATION;
                            else if (line.substring(0,3) == "<td") serviceData = true;
                            else if (line.substring(0,7) == "<a href" && serviceData) {
                                serviceId = stripTag(line);
                                strlcpy(xBusStop->service[id].routeNumber, serviceId.c_str(), sizeof(xBusStop->service[id].routeNumber));
                            } else {
                                serviceId = line;
                                strlcpy(xBusStop->service[id].routeNumber, serviceId.c_str(), sizeof(xBusStop->service[id].routeNumber));
                            }
                            break;
                        case PBT_DESTINATION:
                            if (line.indexOf("</td>") >= 0) parseStep = PBT_SCHEDULED;
                            else if (line.substring(0,1) != "<") {
                                strlcpy(xBusStop->service[id].destination, line.c_str(), sizeof(xBusStop->service[id].destination));
                            } else if (line.indexOf("class=\"vehicle\"") >= 0) {
                                String vehicle = stripTag(line);
                                // The vehicle tag contains " - [NAME]", we only want the name
                                int tikregsep = vehicle.indexOf(" - ");
                                if (tikregsep > 0) {
                                    vehicle = vehicle.substring(tikregsep+3);
                                    vehicle.trim();
                                }
                                if ((strlen(xBusStop->service[id].destination) + vehicle.length() + 3) < BUS_MAX_LOCATION) {
                                    sprintf(xBusStop->service[id].destination, "%s (%s)", xBusStop->service[id].destination, vehicle.c_str());
                                }
                            }
                            break;
                        case PBT_SCHEDULED:
                            if (line.indexOf("</td>") >= 0) {
                                if (dataColumns == 4) parseStep = PBT_EXPECTED; else {
                                    strcpy(xBusStop->service[id].expectedTime, "");
                                    parseStep = PBT_HEADER;
                                    if (serviceMatchesFilter(cleanBusFilter, xBusStop->service[id].routeNumber)) id++;
                                    if (id >= BUS_MAX_SERVICES) maxServicesRead = true;
                                }
                            } else if (line.substring(0,1) != "<") {
                                strlcpy(xBusStop->service[id].sTime, line.c_str(), sizeof(xBusStop->service[id].sTime));
                            }
                            break;
                        case PBT_EXPECTED:
                            if (line.indexOf("</td>") >= 1) {
                                parseStep = PBT_HEADER;
                                if (serviceMatchesFilter(cleanBusFilter, xBusStop->service[id].routeNumber)) id++;
                                if (id >= BUS_MAX_SERVICES) maxServicesRead = true;
                            }
                            else if (line.substring(0,1) != "<") {
                                strlcpy(xBusStop->service[id].expectedTime, line.c_str(), sizeof(xBusStop->service[id].expectedTime));
                            }
                            break;
                    }
                }
            }
            if (millis() > ticker) {
                if (callback) callback();
                ticker = millis() + 800;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10)); // Prevent wild spinning if connection is starved
    }

    // --- Step 5: Finalize and compare results ---
    httpsClient->stop();
    xBusStop->numServices = id;
    
    // --- Step 2: Zero-Copy Position Numbering ---
    // Assign stable pointers to position strings ("1", "2", ...) from the static 
    // library to avoid stack-allocated string corruption in the UI widgets.
    for (int i = 0; i < xBusStop->numServices; i++) {
        xBusStop->service[i].orderNum = serviceNumbers[i % BUS_MAX_FETCH];
        replaceWord(xBusStop->service[i].destination, "&amp;", "&");
    }

    if (stationData) {
        if (xBusStop->numServices != stationData->numServices) stationData->boardChanged = true;
        else {
            for (int i=0; i < (xBusStop->numServices < 2 ? xBusStop->numServices : 2); i++) {
                if (strcmp(xBusStop->service[i].destination, stationData->service[i].destination) || 
                    strcmp(xBusStop->service[i].routeNumber, stationData->service[i].routeNumber)) {
                    stationData->boardChanged = true;
                    break;
                }
            }
        }

        // Apply new data safely
        xSemaphoreTake(dataMutex, portMAX_DELAY);
        stationData->numServices = xBusStop->numServices;
        memcpy(stationData->service, xBusStop->service, sizeof(BusService) * BUS_MAX_SERVICES);
        
        uint32_t hashVal = 2166136261u;
        hashVal = hashPrimitive(stationData->numServices, hashVal);
        hashVal = hashString(stationData->location, hashVal);
        for(int i = 0; i < stationData->numServices; i++) {
            hashVal = hashString(stationData->service[i].sTime, hashVal);
            hashVal = hashString(stationData->service[i].expectedTime, hashVal);
            hashVal = hashString(stationData->service[i].destination, hashVal);
            hashVal = hashString(stationData->service[i].routeNumber, hashVal);
        }
        for(size_t i = 0; i < messagesData.getCount(); i++) {
            hashVal = hashString(messagesData.getMessage(i), hashVal);
        }
        stationData->contentHash = hashVal;

        if (renderData) {
            memcpy(renderData.get(), stationData.get(), sizeof(BusStop));
        }
        renderMessages.clear();
        for (int i = 0; i < messagesData.getCount(); i++) {
            renderMessages.addMessage(messagesData.getMessage(i));
        }
        taskStatus = stationData->boardChanged ? UpdateStatus::SUCCESS : UpdateStatus::NO_CHANGE;
        xSemaphoreGive(dataMutex);

        snprintf(lastErrorMsg, sizeof(lastErrorMsg), "SUCCESS %ums [%ld]", (uint32_t)(millis() - perfTimer), dataReceived);
        LOG_INFOf("DATA", "Bus (ATCO: %s): Found %d services.", busAtco, (int)stationData->numServices);
#ifdef ENABLE_DEBUG_LOG
        UBaseType_t hwm = uxTaskGetStackHighWaterMark(NULL);
        LOG_DEBUG("DATA", "Bus Task Stack High Water Mark: " + String(hwm) + " words");
        LOG_DEBUG("DATA", "--- Bus Data ---");
        for (int i = 0; i < renderData->numServices; i++) {
            // --- Step 3: Populate 4-Column Layout ---
            // Column Order: 0: OrderNum, 1: Route, 2: Destination, 3: Time
            const char* rowData[4] = {
                renderData->service[i].orderNum,
                renderData->service[i].routeNumber,
                renderData->service[i].destination,
                renderData->service[i].expectedTime[0] != '\0' ? renderData->service[i].expectedTime : renderData->service[i].sTime
            };
            char debugMsg[256];
            snprintf(debugMsg, sizeof(debugMsg), "Service %d: [%s] to %s (Sch: %s, Exp: %s)", 
                     i, renderData->service[i].routeNumber, renderData->service[i].destination, 
                     renderData->service[i].sTime, renderData->service[i].expectedTime);
            LOG_DEBUG("DATA", debugMsg);
        }
        LOG_DEBUG("DATA", "----------------");
#endif
        uint32_t interval = BASELINE_MIN_INTERVAL;
        if (renderData && renderData->numServices > 0) interval = 45000;
        setNextFetchTime(millis() + interval);
        LOG_INFOf("DATA", "Bus Source: executeFetch() finished. Next fetch in %dms.", (int)interval);
        goto scrub_and_exit;
    }
    taskStatus = UpdateStatus::DATA_ERROR;
    setNextFetchTime(millis() + BASELINE_MIN_INTERVAL);

scrub_and_exit:
    // [HYBRID-MEM] Scrub the heap before exiting the fetch cycle.
    currentKey.clear();
    currentObject.clear();
    longName.clear();
    return;
}

/**
 * @brief Performs a lightweight connection and authentication test.
 * Bus API currently does not require authentication.
 * @param token Optional token to test (ignored for bus).
 * @param stationId Optional station/stop ID to test.
 * @return UpdateStatus code.
 */
UpdateStatus busDataSource::testConnection(const char* token, const char* stationId) {
    if (stationId && strlen(stationId) > 0) {
        LOG_INFOf("DATA", "Bus Board: Performing station-specific check for ATCO: %s", stationId);
        char prevAtco[13];
        strlcpy(prevAtco, busAtco, sizeof(prevAtco));
        strlcpy(busAtco, stationId, sizeof(busAtco));
        executeFetch();
        UpdateStatus result = taskStatus;
        strlcpy(busAtco, prevAtco, sizeof(busAtco));
        return result;
    }
    LOG_INFO("DATA", "Bus Board: testConnection called without stationId. Bypassing request as no auth required.");
    snprintf(lastErrorMsg, sizeof(lastErrorMsg), "SUCCESS");
    return UpdateStatus::SUCCESS; // bustimes.org doesn't use a token for now, return ok for the UI
}

/**
 * @brief Strips HTML tags from a string snippet.
 * @param html The HTML fragment.
 * @return String The text within the tags.
 */
String busDataSource::stripTag(String html) {
    int start = html.indexOf(">");
    int end = html.indexOf("</");
    if (start != -1 && end != -1 && end > start) {
        String res = html.substring(start+1, end);
        res.trim();
        return res;
    }
    return "";
}

/**
 * @brief In-place replacement of a target string within a char buffer.
 * @param input The buffer to modify.
 * @param target The string to find.
 * @param replacement The string to insert.
 */
void busDataSource::replaceWord(char* input, const char* target, const char* replacement) {
    char* pos = strstr(input, target);
    while (pos) {
        size_t targetLen = strlen(target);
        size_t replacementLen = strlen(replacement);
        // Shift trailing content to fit the new word
        memmove(pos + replacementLen, pos + targetLen, strlen(pos + targetLen) + 1);
        memcpy(pos, replacement, replacementLen);
        pos = strstr(pos + replacementLen, target);
    }
}

/**
 * @brief Trims whitespace from start and end of a char sequence.
 * @param start Reference to start pointer.
 * @param end Reference to end pointer.
 */
void busDataSource::trim(char* &start, char* &end) {
    while (start <= end && isspace(*start)) start++;
    while (end >= start && isspace(*end)) end--;
}

/**
 * @brief Case-insensitive comparison of two string buffers.
 * @param a First string.
 * @param a_len Length of first string.
 * @param b Second string.
 * @return true if equal.
 */
bool busDataSource::equalsIgnoreCase(const char* a, int a_len, const char* b) {
    for (int i = 0; i < a_len; i++) {
        if (tolower(a[i]) != tolower(b[i])) return false;
    }
    return b[a_len] == '\0';
}

/**
 * @brief Checks if a service matches the route filter list.
 * @param filter CSV list of routes.
 * @param serviceId The service to check.
 * @return true if filtering is off or service is in the list.
 */
bool busDataSource::serviceMatchesFilter(const char* filter, const char* serviceId) {
    if (!filter || filter[0] == '\0') return true;
    const char* start = filter;
    const char* ptr = filter;
    while (true) {
        if (*ptr == ',' || *ptr == '\0') {
            const char* end = ptr - 1;
            char* tS = const_cast<char*>(start);
            char* tE = const_cast<char*>(end);
            trim(tS, tE);
            int len = tE - tS + 1;
            if (len > 0 && equalsIgnoreCase(tS, len, serviceId)) return true;
            if (*ptr == '\0') break;
            start = ++ptr;
        } else ptr++;
    }
    return false;
}

/**
 * @brief Cleans a raw filter string (lowercase, remove spaces).
 * @param rawFilter The raw filter input.
 * @param cleanedFilter Buffer for the cleaned filter.
 * @param maxLen Maximum length of the buffer.
 */
void busDataSource::cleanFilter(const char* rawFilter, char* cleanedFilter, size_t maxLen) {
    if (!rawFilter || rawFilter[0] == '\0') {
        if (maxLen > 0) cleanedFilter[0] = '\0';
        return;
    }
    size_t j = 0;
    const char* ptr = rawFilter;
    while (*ptr != '\0' && j < maxLen - 1) {
        if (*ptr == ',') cleanedFilter[j++] = ',';
        else if (!isspace(*ptr)) cleanedFilter[j++] = tolower(*ptr);
        ptr++;
    }
    cleanedFilter[j] = '\0';
}

void busDataSource::whitespace(char c) {}
void busDataSource::startDocument() {}
void busDataSource::key(String key) { 
    // [HYBRID-MEM] String used for flexibility in key matching.
    currentKey = key; 
}
void busDataSource::value(String value) { 
    if (currentKey == "long_name") longName = value; 
}
void busDataSource::endArray() {}
void busDataSource::endObject() {}
void busDataSource::endDocument() {}
void busDataSource::startArray() {}
void busDataSource::startObject() {}

/**
 * @brief Fetches friendly stop name using the bustimes JSON API.
 * @param locationId ATCO code.
 * @param locationName Buffer to store result.
 * @return UpdateStatus code.
 */
UpdateStatus busDataSource::getStopLongName(const char *locationId, char *locationName) {
    std::unique_ptr<JsonStreamingParser> parser(new (std::nothrow) JsonStreamingParser());
    std::unique_ptr<WiFiClientSecure> httpsClient(new (std::nothrow) WiFiClientSecure());
    if (!parser || !httpsClient) {
        LOG_ERROR("DATA", "Bus Board: Memory allocation failed for getStopLongName!");
        return UpdateStatus::DATA_ERROR;
    }

    parser->setListener(this);
    httpsClient->setInsecure();
    if (!httpsClient->connect(apiHost, 443)) return UpdateStatus::NO_RESPONSE;
    
    char request[256];
    int len = snprintf(request, sizeof(request), "GET /api/stops/%s HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n", locationId, apiHost);
    
    if (len >= (int)sizeof(request)) {
        LOG_ERRORf("DATA", "Bus Source: getStopLongName URL too long! (%d bytes)", len);
        httpsClient->stop();
        return UpdateStatus::DATA_ERROR;
    }
    
    httpsClient->print(request);
    while (httpsClient->connected() || httpsClient->available()) {
        if (httpsClient->available()) parser->parse(httpsClient->read());
        else delay(10);
    }
    httpsClient->stop();
    strlcpy(locationName, longName.c_str(), 80); // locationName is size 80 usually

    // [HYBRID-MEM] Scrub strings
    currentKey.clear();
    longName.clear();

    return UpdateStatus::SUCCESS;
}

void busDataSource::serializeData(JsonObject& doc) {
    lockData();
    doc["atco"] = busAtco;
    
    JsonArray services = doc["departures"].to<JsonArray>();
    for (int i = 0; i < renderData->numServices; i++) {
        JsonObject s = services.add<JsonObject>();
        s["route"] = renderData->service[i].routeNumber;
        s["destination"] = renderData->service[i].destination;
        s["expected"] = (renderData->service[i].expectedTime[0] != '\0') ? renderData->service[i].expectedTime : renderData->service[i].sTime;
    }
    unlockData();
}
