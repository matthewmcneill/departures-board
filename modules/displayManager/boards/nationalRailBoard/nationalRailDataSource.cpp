/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/boards/nationalRailBoard/nationalRailDataSource.cpp
 * Description: Implementation of National Rail data source ported from raildataXmlClient.
 *
 * Exported Functions/Classes:
 * - nationalRailDataSource: Data client for Darwin (National Rail SOAP API).
 *   - init(): Initializes the WSDL parser and SSL handshake.
 *   - updateData(): High-level trigger for polling departure info.
 *   - getStationData(): Returns parsed station name and meta.
 *   - getMessagesData(): Accessor for rail disruption messages.
 *   - setSoapAddress(): Manually set SOAP endpoint (bypasses WSDL).
 *   - configure(): Set API token and station parameters.
 */

#include "nationalRailDataSource.hpp"
#include <WiFiClientSecure.h>
#include <logger.hpp>
#include "../xmlStreamingParser/xmlStreamingParser.hpp"
#include <algorithm>
#include <appContext.hpp>

#include "../interfaces/iDataSource.hpp"

extern class appContext appContext;

nationalRailDataSource::nationalRailDataSource() 
    : loadingWDSL(false), tagLevel(0), isTestMode(false), id(-1), coaches(0), addedStopLocation(false), 
      filterPlatforms(false), keepRoute(false), nrTimeOffset(0), callback(nullptr),
      messagesData(4), renderMessages(4) {
    stationData = std::unique_ptr<NationalRailStation>(new (std::nothrow) NationalRailStation());
    renderData = std::unique_ptr<NationalRailStation>(new (std::nothrow) NationalRailStation());
    if (stationData) memset(stationData.get(), 0, sizeof(NationalRailStation));
    if (renderData) memset(renderData.get(), 0, sizeof(NationalRailStation));
    
    dataMutex = xSemaphoreCreateMutex();
    taskStatus = UPD_NO_DATA;
    
    lastErrorMessage[0] = '\0';
    soapHost[0] = '\0';
    soapAPI[0] = '\0';
    nrToken[0] = '\0';
    crsCode[0] = '\0';
    platformFilter[0] = '\0';
    callingCrsCode[0] = '\0';
}

void nationalRailDataSource::configure(const char* token, const char* crs, const char* filter, const char* callingCrs, int offset) {
    if (token) strlcpy(nrToken, token, sizeof(nrToken));
    if (crs) strlcpy(crsCode, crs, sizeof(crsCode));
    if (filter) strlcpy(platformFilter, filter, sizeof(platformFilter));
    if (callingCrs) strlcpy(callingCrsCode, callingCrs, sizeof(callingCrsCode));
    nrTimeOffset = offset;
    filterPlatforms = (platformFilter[0] != '\0');
}

bool nationalRailDataSource::compareTimes(const NationalRailService& a, const NationalRailService& b) {
    int h1, m1, h2, m2;
    sscanf(a.sTime, "%d:%d", &h1, &m1);
    sscanf(b.sTime, "%d:%d", &h2, &m2);
    if (h1 != h2) {
        if (h1 < 2 && h2 > 20) return false;
        if (h2 < 2 && h1 > 20) return true;
        return h1 < h2;
    }
    return m1 < m2;
}

int nationalRailDataSource::init(const char *wsdlHost, const char *wsdlAPI, nrDataSourceCallback cb) {
    LOG_INFO("DATA", "NR Source: Initializing Darwin API...");
    callback = cb;
    
    WiFiClientSecure *httpsClient = new (std::nothrow) WiFiClientSecure();
    if (!httpsClient) {
        LOG_ERROR("DATA", "NR Source: Failed to allocate WiFiClientSecure on heap!");
        return UPD_DATA_ERROR;
    }

    httpsClient->setInsecure();
    httpsClient->setTimeout(5000);
    httpsClient->setConnectionTimeout(3000);

    int retry = 0;
    while(!httpsClient->connect(wsdlHost, 443) && (retry < 10)){
        delay(100); retry++;
    }
    if(retry >= 10) {
        LOG_ERROR("DATA", "NR Source: Failed to connect to WSDL host: " + String(wsdlHost));
        snprintf(lastErrorMessage, sizeof(lastErrorMessage), "WSDL connection failed");
        delete httpsClient;
        return UPD_NO_RESPONSE;
    }

    LOG_INFO("DATA", "NR Source: Connected to WSDL host. Requesting " + String(wsdlAPI));
    httpsClient->print("GET " + String(wsdlAPI) + " HTTP/1.0\r\nHost: " + String(wsdlHost) + "\r\nConnection: close\r\n\r\n");

    retry = 0;
    while(!httpsClient->available() && retry < 100) { delay(100); retry++; }
    if (retry >= 100) {
        delete httpsClient;
        return UPD_TIMEOUT;
    }

    while (httpsClient->connected() || httpsClient->available()) {
        String line = httpsClient->readStringUntil('\n');
        if (line.startsWith("HTTP") && line.indexOf("200 OK") == -1) {
            httpsClient->stop();
            delete httpsClient;
            return UPD_HTTP_ERROR;
        }
        if (line == "\r") break;
    }

    loadingWDSL = true;
    xmlStreamingParser parser;
    parser.setListener(this);
    parser.reset();
    grandParentTagName = ""; parentTagName = ""; tagName = ""; tagLevel = 0;

    while(httpsClient->available() || httpsClient->connected()) {
        while (httpsClient->available()) parser.parse(httpsClient->read());
    }
    httpsClient->stop();
    delete httpsClient;
    loadingWDSL = false;

    if (soapURL.startsWith("https://")) {
        int delim = soapURL.indexOf("/", 8);
        if (delim > 0) {
            soapURL.substring(8, delim).toCharArray(soapHost, sizeof(soapHost));
            soapURL.substring(delim).toCharArray(soapAPI, sizeof(soapAPI));
            return UPD_SUCCESS;
        }
    }
    return UPD_DATA_ERROR;
}

void nationalRailDataSource::setSoapAddress(const char* host, const char* api) {
    if (host) strlcpy(soapHost, host, sizeof(soapHost));
    if (api) strlcpy(soapAPI, api, sizeof(soapAPI));
    isTestMode = true;
    LOG_INFO("DATA", "NR Source: Soap address manually set to " + String(soapHost) + String(soapAPI) + " (Test Mode enabled)");
}

int nationalRailDataSource::updateData() {
    if (taskStatus == UPD_PENDING) {
        return UPD_PENDING;
    }
    
    LOG_INFO("DATA", "NR Source: Enqueuing fetch to DataWorker");
    taskStatus = UPD_PENDING;
    appContext.getDataWorker().enqueueRequest(this);
    return UPD_PENDING;
}

/**
 * @brief Internal blocking method that executes the SOAP protocol and coordinates XML streaming parse.
 */
void nationalRailDataSource::executeFetch() {
    LOG_INFO("DATA", "NR Source: executeFetch() entry. Free heap: " + String(ESP.getFreeHeap()));
    unsigned long perfTimer = millis();
    bool bChunked = false;
    lastErrorMessage[0] = '\0';

    id = -1; coaches = 0; addedStopLocation = false; keepRoute = false;

    WiFiClientSecure *httpsClient = new (std::nothrow) WiFiClientSecure();
    if (!httpsClient) {
        LOG_ERROR("DATA", "NR Source: Failed to allocate WiFiClientSecure for update!");
        taskStatus = UPD_DATA_ERROR;
        return;
    }
    httpsClient->setInsecure();
    httpsClient->setTimeout(8000); // Increased timeout
    httpsClient->setConnectionTimeout(8000);

    LOG_INFO("DATA", "NR Source: Connecting to [" + String(soapHost) + "] Path: [" + String(soapAPI) + "]. Heap: " + String(ESP.getFreeHeap()));
    
    int soapRetry = 0;
    while (!httpsClient->connect(soapHost, 443) && soapRetry < 5) {
        LOG_WARN("DATA", String("NR Source: Connection attempt ") + soapRetry + " failed. Retrying...");
        delay(300);
        soapRetry++;
    }

    if(soapRetry >= 5) {
        LOG_ERROR("DATA", "NR Source: Connection failed after retries!");
        snprintf(lastErrorMessage, sizeof(lastErrorMessage), "SOAP connection failed");
        httpsClient->stop();
        delete httpsClient;
        taskStatus = UPD_NO_RESPONSE;
        return;
    }

    LOG_INFO("DATA", "NR Source: Connection established. Free heap: " + String(ESP.getFreeHeap()));

    // Allocate temporary stations on the heap AFTER SSL link is up to conserve peak heap
    NationalRailStation* xStation = new (std::nothrow) NationalRailStation();
    if (!xStation) {
        LOG_ERROR("DATA", "NR Source: Failed to allocate memory for station data update!");
        httpsClient->stop();
        delete httpsClient;
        taskStatus = UPD_DATA_ERROR;
        return;
    }
    memset(xStation, 0, sizeof(NationalRailStation));

    String data;
    String soapAction;

    if (isTestMode) {
        // Minimal Darwin v12 payload for fast API key validation
        data = F("<Envelope xmlns=\"http://schemas.xmlsoap.org/soap/envelope/\">");
        data += F("<Header><AccessToken xmlns=\"http://thalesgroup.com/RTTI/2013-11-28/Token/types\"><TokenValue>");
        data += String(nrToken) + F("</TokenValue></AccessToken></Header>");
        data += F("<Body><GetDepartureBoardRequest xmlns=\"http://thalesgroup.com/RTTI/2021-11-01/ldb/\"><numRows>1</numRows><crs>");
        data += String(crsCode) + F("</crs></GetDepartureBoardRequest></Body></Envelope>");
        soapAction = F("http://thalesgroup.com/RTTI/2012-01-13/ldb/GetDepartureBoard");
        LOG_INFO("DATA", "NR Source: Using lightweight v12 validation payload");
    } else {
        int reqRows = NR_MAX_SERVICES;
        if (platformFilter[0]) reqRows = 10;
        
        data = F("<soap-env:Envelope xmlns:soap-env=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:ns1=\"http://thalesgroup.com/RTTI/2013-11-28/Token/types\" xmlns:ns2=\"http://thalesgroup.com/RTTI/2021-11-01/ldb/\">");
        data += F("<soap-env:Header><ns1:AccessToken><ns1:TokenValue>");
        data += String(nrToken) + F("</ns1:TokenValue></ns1:AccessToken></soap-env:Header><soap-env:Body><ns2:GetDepBoardWithDetailsRequest><ns2:numRows>");
        data += String(reqRows) + F("</ns2:numRows><ns2:crs>");
        data += String(crsCode) + F("</ns2:crs>");
        if (callingCrsCode[0]) data += "<ns2:filterCrs>" + String(callingCrsCode) + F("</ns2:filterCrs><ns2:filterType>to</ns2:filterType>");
        if (nrTimeOffset) data += "<ns2:timeOffset>" + String(nrTimeOffset) + F("</ns2:timeOffset>");
        data += F("</ns2:GetDepBoardWithDetailsRequest></soap-env:Body></soap-env:Envelope>");

        soapAction = F("http://thalesgroup.com/RTTI/2015-05-14/ldb/GetDepBoardWithDetails");
    }
    
    // Diagnostic logging for token
    if (strlen(nrToken) > 0) {
        String t = String(nrToken);
        String preview = t.substring(0, 4) + "..." + t.substring(t.length() - 4);
        LOG_INFO("DATA", "NR Source: Sending request with token: " + preview + " (Len: " + String(t.length()) + ")");
    } else {
        LOG_WARN("DATA", "NR Source: Token is EMPTY!");
    }

    String requestBody = "POST " + String(soapAPI) + " HTTP/1.1\r\n" +
                         "Host: " + String(soapHost) + "\r\n" +
                         "Content-Type: text/xml; charset=utf-8\r\n" +
                         "SOAPAction: \"" + soapAction + "\"\r\n" +
                         "User-Agent: ESP32/1.0\r\n" +
                         "Connection: close\r\n" +
                         "Content-Length: " + String(data.length()) + "\r\n\r\n" + data;

    // Log the request for debugging (mask token)
    String debugReq = requestBody;
    int tokenIdx = debugReq.indexOf(nrToken);
    if (tokenIdx >= 0) {
        debugReq = debugReq.substring(0, tokenIdx) + "********" + debugReq.substring(tokenIdx + strlen(nrToken));
    }
    LOG_DEBUG("DATA", "NR Request:\n" + debugReq);

    httpsClient->print(requestBody);

    if (callback) callback(1, 0);
    
    unsigned long ticker = millis() + 350;
    int retry = 0;
    while(!httpsClient->available() && retry < 30) { delay(100); retry++; }
    if (retry >= 30) {
        LOG_ERROR("DATA", "NR Source: Request timeout!");
        delete xStation;
        taskStatus = UPD_TIMEOUT;
        return;
    }

    while (httpsClient->connected() || httpsClient->available()) {
        String line = httpsClient->readStringUntil('\n');
        line.trim();
        if (line.startsWith("HTTP")) {
            LOG_INFO("DATA", "NR Source: HTTP Response: " + line);
            if (line.indexOf("200 OK") == -1) {
                LOG_ERROR("DATA", "NR Source: HTTP Error detected: " + line);
                snprintf(lastErrorMessage, sizeof(lastErrorMessage), "HTTP Error: %s", line.c_str());
                
                // Read and log ALL headers
                while (httpsClient->connected() || httpsClient->available()) {
                    String hLine = httpsClient->readStringUntil('\n');
                    hLine.trim();
                    if (hLine.length() == 0) break;
                    LOG_ERROR("DATA", "NR Header: " + hLine);
                }

                // Now read the body (SOAP Fault)
                String faultBody = "";
                for(int i=0; i<15 && httpsClient->available(); i++) {
                    faultBody += httpsClient->readStringUntil('\n');
                }
                LOG_ERROR("DATA", "NR Source: Error Body: " + faultBody.substring(0, 300));
                
                httpsClient->stop();
                delete httpsClient;
                delete xStation;
                taskStatus = UPD_HTTP_ERROR;
                return;
            }
        } else if (line.startsWith("Transfer-Encoding:") && line.indexOf("chunked") >= 0) {
            bChunked = true;
        }
        if (line.length() == 0) break;
    }

    xmlStreamingParser parser;
    parser.setListener(this);
    parser.reset();
    grandParentTagName = ""; parentTagName = ""; tagName = ""; tagLevel = 0; loadingWDSL = false;
    long bytesRecv = 0;

    // Temporarily point stationData to xStation during parsing
    if (stationData && xStation) *stationData = *xStation;

    // We'll clear messagesData and populate it directly during parsing
    messagesData.clear();

    LOG_INFO("DATA", "NR Source: Starting XML parse...");
    int yieldCounter = 0;
    while(httpsClient->available() || httpsClient->connected()) {
        while (httpsClient->available()) {
            parser.parse(httpsClient->read());
            bytesRecv++;
            
            // --- Arcane Logic ---
            // On single-core ESP32 variants (e.g. ESP32-C3), the Wi-Fi stack and user application
            // share the exact same processor core tightly via the RTOS scheduler. By explicitly
            // yielding execution context via vTaskDelay(1) every 500 byte blocks, we guarantee
            // network hardware interrupts service without triggering Task Watchdog Timers (TWDT).
            yieldCounter++;
            if (yieldCounter % 500 == 0) vTaskDelay(1); // Single-core compatibility yield
            
            if (millis() > ticker && stationData) {
                if (callback) callback(2, stationData->numServices);
                ticker = millis() + 350;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    httpsClient->stop();
    delete httpsClient;
    LOG_INFO("DATA", "NR Source: XML parse complete. Bytes: " + String(bytesRecv));

    // After parsing, sanitize and check changes
    sanitiseData();
    
    // For now, always treat a successful parse as a change to trigger board refresh
    // Simplifies memory and is generally what's needed for departures
    if (stationData) {
        stationData->boardChanged = true;
    }

    // Critical Double Buffering Swap
    xSemaphoreTake(dataMutex, portMAX_DELAY);
    if (stationData && renderData) {
        *renderData = *stationData;
    }
    renderMessages.clear();
    for (int i = 0; i < messagesData.getCount(); i++) {
        renderMessages.addMessage(messagesData.getMessage(i));
    }
    taskStatus = UPD_SUCCESS;
    xSemaphoreGive(dataMutex);

    if (callback && renderData) callback(3, renderData->numServices);
    snprintf(lastErrorMessage, sizeof(lastErrorMessage), "SUCCESS [%ld] %lums", bytesRecv, (unsigned long)(millis()-perfTimer));
    
    delete xStation;
    
    LOG_INFO("DATA", "NR Source: executeFetch() finished successfully.");
#ifdef ENABLE_DEBUG_LOG
    UBaseType_t hwm = uxTaskGetStackHighWaterMark(NULL);
    LOG_DEBUG("DATA", "NR Task Stack High Water Mark: " + String(hwm) + " words");
    if (renderData) {
        LOG_DEBUG("DATA", "--- National Rail Data ---");
        LOG_DEBUG("DATA", String("Location: ") + renderData->location);
        for (int i = 0; i < renderData->numServices; i++) {
            char debugMsg[256];
            snprintf(debugMsg, sizeof(debugMsg), "Service %d: %s to %s (Exp: %s, Plat: %s)", 
                     i, renderData->service[i].sTime, renderData->service[i].destination, 
                     renderData->service[i].etd, renderData->service[i].platform);
            LOG_DEBUG("DATA", debugMsg);
        }
        LOG_DEBUG("DATA", "--------------------------");
    }
#endif
    return;
}

/**
 * @brief Performs a lightweight connection and authentication test using a minimal payload.
 * @param token Optional token to test (overrides stored configuration).
 * @return int Update status code.
 */
int nationalRailDataSource::testConnection(const char* token, const char* stationId) {
    LOG_INFO("DATA", "NR Source: Performing lightweight Auth-only check via testConnection");
    
    // Save current state to avoid clobbering an active board's settings
    bool prevTestMode = isTestMode;
    char prevKey[37];
    char prevCrs[4];
    char prevSoapHost[48];
    char prevSoapApi[48];
    
    strlcpy(prevKey, nrToken, sizeof(prevKey));
    strlcpy(prevCrs, crsCode, sizeof(prevCrs));
    strlcpy(prevSoapHost, soapHost, sizeof(prevSoapHost));
    strlcpy(prevSoapApi, soapAPI, sizeof(prevSoapApi));
    
    // Configure for test
    isTestMode = true;
    if (token) {
        strlcpy(nrToken, token, sizeof(nrToken));
    }
    
    if (stationId && strlen(stationId) > 0) {
        strlcpy(crsCode, stationId, sizeof(crsCode));
    } else {
        strlcpy(crsCode, "PAD", sizeof(crsCode)); // Paddington as default test station
    }
    
    // Direct SOAP setup bypasses WSDL download.
    // Using ldb12.asmx for the identified minimal payload compatibility.
    setSoapAddress("lite.realtime.nationalrail.co.uk", "/OpenLDBWS/ldb12.asmx");
    
    // Execute update (in test mode it uses minimal payload)
    // Synchronously execute it for testConnection
    executeFetch();
    int result = taskStatus;
    
    // Restore state
    isTestMode = prevTestMode;
    strlcpy(nrToken, prevKey, sizeof(nrToken));
    strlcpy(crsCode, prevCrs, sizeof(crsCode));
    strlcpy(soapHost, prevSoapHost, sizeof(soapHost));
    strlcpy(soapAPI, prevSoapApi, sizeof(soapAPI));
    
    // Convert UPD_NO_CHANGE (1) to UPD_SUCCESS (0) for test result consistency since we don't care about board state changing
    if (result == UPD_NO_CHANGE) result = UPD_SUCCESS;
    
    return result;
}

void nationalRailDataSource::sanitiseData() {
    if (!stationData) return;
    // Basic sanitization from raildataXmlClient
    removeHtmlTags(stationData->location);
    replaceWord(stationData->location, "&amp;", "&");
    for (int i=0; i<stationData->numServices; i++) {
        removeHtmlTags(stationData->service[i].destination);
        replaceWord(stationData->service[i].destination, "&amp;", "&");
        removeHtmlTags(stationData->service[i].calling);
        replaceWord(stationData->service[i].calling, "&amp;", "&");
        removeHtmlTags(stationData->service[i].via);
        replaceWord(stationData->service[i].via, "&amp;", "&");
        removeHtmlTags(stationData->service[i].serviceMessage);
        replaceWord(stationData->service[i].serviceMessage, "&amp;", "&");
        fixFullStop(stationData->service[i].serviceMessage);
    }
    // Note: Sanitization for MessagePool strings happens during addition or after?
    // Current Darwin logic adds them. We'll leave them as is for now or add a sanitize method to Pool.
}

void nationalRailDataSource::removeHtmlTags(char* input) {
    bool inTag = false; char* out = input;
    for (char* p = input; *p; p++) {
        if (*p == '<') inTag = true;
        else if (*p == '>') inTag = false;
        else if (!inTag) *out++ = *p;
    }
    *out = '\0';
}

void nationalRailDataSource::replaceWord(char* input, const char* target, const char* replacement) {
    char* pos = strstr(input, target);
    while (pos) {
        size_t tLen = strlen(target);
        size_t rLen = strlen(replacement);
        memmove(pos + rLen, pos + tLen, strlen(pos + tLen) + 1);
        memcpy(pos, replacement, rLen);
        pos = strstr(pos + rLen, target);
    }
}

void nationalRailDataSource::pruneFromPhrase(char* input, const char* target) {
    char* pos = strstr(input, target);
    if (pos) *pos = '\0';
}

void nationalRailDataSource::fixFullStop(char* input) {
    if (input[0]) {
        size_t len = strlen(input);
        while (len > 0 && (input[len-1] == '.' || input[len-1] == ' ')) input[--len] = '\0';
        if (len < NR_MAX_MSG_LEN - 1) strcat(input, ".");
    }
}

void nationalRailDataSource::trim(char* &start, char* &end) {
    while (start <= end && isspace(*start)) start++;
    while (end >= start && isspace(*end)) end--;
}

bool nationalRailDataSource::equalsIgnoreCase(const char* a, int a_len, const char* b) {
    for (int i = 0; i < a_len; i++) if (tolower(a[i]) != tolower(b[i])) return false;
    return b[a_len] == '\0';
}

bool nationalRailDataSource::serviceMatchesFilter(const char* filter, const char* serviceId) {
    if (!filter || !filter[0]) return true;
    const char* start = filter; const char* ptr = filter;
    while (true) {
        if (*ptr == ',' || *ptr == '\0') {
            const char* end = ptr - 1;
            char* tS = const_cast<char*>(start); char* tE = const_cast<char*>(end);
            trim(tS, tE);
            int len = tE - tS + 1;
            if (len > 0 && equalsIgnoreCase(tS, len, serviceId)) return true;
            if (*ptr == '\0') break;
            start = ++ptr;
        } else ptr++;
    }
    return false;
}

void nationalRailDataSource::deleteService(int x) {
    if (!stationData || x < 0 || x >= stationData->numServices) return;
    for (int i=x; i<stationData->numServices-1; i++) stationData->service[i] = stationData->service[i+1];
    stationData->numServices--;
}

void nationalRailDataSource::startTag(const char *tag) {
    tagLevel++; grandParentTagName = parentTagName; parentTagName = tagName; tagName = tag;
    tagPath = grandParentTagName + "/" + parentTagName + "/" + tagName;
}

void nationalRailDataSource::endTag(const char *tag) {
    tagLevel--; tagName = parentTagName; parentTagName = grandParentTagName; grandParentTagName = "??";
}

void nationalRailDataSource::parameter(const char *param) {}

void nationalRailDataSource::value(const char *val) {
    if (loadingWDSL) return;
    if (tagLevel < 6) return;

    if (tagLevel == 11 && tagPath.endsWith("callingPoint/lt8:locationName")) {
        if (id >= 0 && stationData && (strlen(stationData->service[id].calling) + strlen(val) + 10) < NR_MAX_CALLING) {
            if (stationData->service[id].calling[0]) strcat(stationData->service[id].calling, ", ");
            strcat(stationData->service[id].calling, val);
            addedStopLocation = true;
        }
    } else if (tagLevel == 11 && tagPath.endsWith("callingPoint/lt8:st") && addedStopLocation) {
        if (id >= 0 && stationData && (strlen(stationData->service[id].calling) + strlen(val) + 4) < NR_MAX_CALLING) {
            strcat(stationData->service[id].calling, " (");
            strcat(stationData->service[id].calling, val);
            strcat(stationData->service[id].calling, ")");
        }
        addedStopLocation = false;
    } else if (tagLevel == 8 && tagName == "lt4:operator") {
        if (id >= 0 && stationData) strncpy(stationData->service[id].opco, val, 49);
    } else if (tagLevel == 8 && tagName == "lt4:std") {
        if (id < NR_MAX_SERVICES - 1 && stationData) { 
            id++; 
            stationData->numServices++;
            strncpy(stationData->service[id].sTime, val, 5);
        }
    } else if (tagLevel == 8 && tagName == "lt4:etd") {
        if (id >= 0 && stationData) strncpy(stationData->service[id].etd, val, 10);
    } else if (tagLevel == 10 && tagPath.indexOf("destination") != -1 && tagPath.endsWith("locationName")) {
        if (id >= 0 && stationData) strncpy(stationData->service[id].destination, val, NR_MAX_LOCATION-1);
    } else if (tagLevel == 10 && tagPath.indexOf("destination") != -1 && tagPath.endsWith("via")) {
        if (id >= 0 && stationData) strncpy(stationData->service[id].via, val, NR_MAX_LOCATION-1);
    } else if (tagLevel == 8 && tagName == "lt4:platform") {
        if (id >= 0 && stationData) {
            strncpy(stationData->service[id].platform, val, 3);
            if (filterPlatforms && !serviceMatchesFilter(platformFilter, val)) {
                // Technically Darwin client shuffles this, but here we just flag it
            }
        }
    } else if (tagLevel == 6 && tagName == "lt4:locationName") {
        if (stationData) strncpy(stationData->location, val, NR_MAX_LOCATION-1);
    } else if (tagPath.endsWith("nrccMessages/lt:message")) {
        messagesData.addMessage(val);
    }
}

void nationalRailDataSource::attribute(const char *attr) {
    if (loadingWDSL && tagName == "soap:address") {
        String s = attr;
        int start = s.indexOf("location=\"");
        if (start != -1) {
            int end = s.indexOf("\"", start+10);
            if (end != -1) soapURL = s.substring(start+10, end);
        }
    }
}
