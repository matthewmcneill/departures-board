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
 *   - getStation(): Returns parsed station name and meta.
 *   - getServicesCount(): Returns current found services.
 *   - getService(): Accessor for individual service rows.
 *   - getMessagesCount(): Accessor for rail disruption messages.
 */

#include "nationalRailDataSource.hpp"
#include <WiFiClientSecure.h>
#include <logger.hpp>
#include "../xmlStreamingParser/xmlStreamingParser.hpp"
#include <algorithm>

#include "../interfaces/iDataSource.hpp"

nationalRailDataSource::nationalRailDataSource() 
    : loadingWDSL(false), tagLevel(0), id(-1), coaches(0), addedStopLocation(false), 
      filterPlatforms(false), keepRoute(false), nrTimeOffset(0), callback(nullptr),
      messagesData(4) {
    stationData = std::unique_ptr<NationalRailStation>(new (std::nothrow) NationalRailStation());
    if (stationData) memset(stationData.get(), 0, sizeof(NationalRailStation));
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
    LOG_INFO("DATA", "NR Source: Soap address manually set to " + String(soapHost) + String(soapAPI));
}

int nationalRailDataSource::updateData() {
    LOG_INFO("DATA", "NR Source: updateData() entry. Free heap: " + String(ESP.getFreeHeap()));
    unsigned long perfTimer = millis();
    bool bChunked = false;
    lastErrorMessage[0] = '\0';

    id = -1; coaches = 0; addedStopLocation = false; keepRoute = false;

    WiFiClientSecure *httpsClient = new (std::nothrow) WiFiClientSecure();
    if (!httpsClient) {
        LOG_ERROR("DATA", "NR Source: Failed to allocate WiFiClientSecure for update!");
        return UPD_DATA_ERROR;
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
        return UPD_NO_RESPONSE;
    }

    LOG_INFO("DATA", "NR Source: Connection established. Free heap: " + String(ESP.getFreeHeap()));

    // Allocate temporary stations on the heap AFTER SSL link is up to conserve peak heap
    NationalRailStation* xStation = new (std::nothrow) NationalRailStation();
    if (!xStation) {
        LOG_ERROR("DATA", "NR Source: Failed to allocate memory for station data update!");
        httpsClient->stop();
        delete httpsClient;
        return UPD_DATA_ERROR;
    }
    memset(xStation, 0, sizeof(NationalRailStation));

    int reqRows = NR_MAX_SERVICES;
    if (platformFilter[0]) reqRows = 10;
    
    String data = F("<soap-env:Envelope xmlns:soap-env=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:ns1=\"http://thalesgroup.com/RTTI/2013-11-28/Token/types\" xmlns:ns2=\"http://thalesgroup.com/RTTI/2021-11-01/ldb/\">");
    data += F("<soap-env:Header><ns1:AccessToken><ns1:TokenValue>");
    data += String(nrToken) + F("</ns1:TokenValue></ns1:AccessToken></soap-env:Header><soap-env:Body><ns2:GetDepBoardWithDetailsRequest><ns2:numRows>");
    data += String(reqRows) + F("</ns2:numRows><ns2:crs>");
    data += String(crsCode) + F("</ns2:crs>");
    if (callingCrsCode[0]) data += "<ns2:filterCrs>" + String(callingCrsCode) + F("</ns2:filterCrs><ns2:filterType>to</ns2:filterType>");
    if (nrTimeOffset) data += "<ns2:timeOffset>" + String(nrTimeOffset) + F("</ns2:timeOffset>");
    data += F("</ns2:GetDepBoardWithDetailsRequest></soap-env:Body></soap-env:Envelope>");

    String soapAction = F("http://thalesgroup.com/RTTI/2012-01-13/ldb/GetDepBoardWithDetails");
    
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
        return UPD_TIMEOUT;
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
                return UPD_HTTP_ERROR;
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
    while(httpsClient->available() || httpsClient->connected()) {
        while (httpsClient->available()) {
            parser.parse(httpsClient->read());
            bytesRecv++;
            if (millis() > ticker && stationData) {
                if (callback) callback(2, stationData->numServices);
                ticker = millis() + 350;
            }
        }
        delay(5);
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

    if (callback && stationData) callback(3, stationData->numServices);
    snprintf(lastErrorMessage, sizeof(lastErrorMessage), "SUCCESS [%ld] %lums", bytesRecv, (unsigned long)(millis()-perfTimer));
    
    delete xStation;
    
    LOG_INFO("DATA", "NR Source: updateData() finished successfully.");
    if (stationData) {
        LOG_INFO("DATA", "NR (CRS: " + String(crsCode) + "): Found " + String(stationData->numServices) + " services.");
#ifdef ENABLE_DEBUG_LOG
        LOG_DEBUG("DATA", "--- National Rail Data ---");
        LOG_DEBUG("DATA", String("Location: ") + stationData->location);
        for (int i = 0; i < stationData->numServices; i++) {
            char debugMsg[256];
            snprintf(debugMsg, sizeof(debugMsg), "Service %d: %s to %s (Exp: %s, Plat: %s)", 
                     i, stationData->service[i].sTime, stationData->service[i].destination, 
                     stationData->service[i].etd, stationData->service[i].platform);
            LOG_DEBUG("DATA", debugMsg);
        }
        LOG_DEBUG("DATA", "--------------------------");
#endif
    }
    return UPD_SUCCESS;
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
