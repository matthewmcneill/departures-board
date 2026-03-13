/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/boards/nationalRailBoard/src/nationalRailDataSource.cpp
 * Description: Implementation of National Rail data source ported from raildataXmlClient.
 */

#include "nationalRailDataSource.hpp"
#include <WiFiClientSecure.h>
#include <Logger.hpp>
#include "../xmlStreamingParser/xmlStreamingParser.h"
#include <algorithm>

#include "../interfaces/iDataSource.hpp"

nationalRailDataSource::nationalRailDataSource() 
    : loadingWDSL(false), tagLevel(0), id(-1), coaches(0), addedStopLocation(false), 
      filterPlatforms(false), keepRoute(false), nrTimeOffset(0), callback(nullptr) {
    memset(&stationData, 0, sizeof(NationalRailStation));
    memset(&messagesData, 0, sizeof(stnMessages));
    lastErrorMessage[0] = '\0';
    soapHost[0] = '\0';
    soapAPI[0] = '\0';
    nrToken[0] = '\0';
    crsCode[0] = '\0';
    platformFilter[0] = '\0';
    callingCrsCode[0] = '\0';
}

void nationalRailDataSource::configure(const char* token, const char* crs, const char* filter, const char* callingCrs, int offset) {
    if (token) strncpy(nrToken, token, sizeof(nrToken)-1);
    if (crs) strncpy(crsCode, crs, sizeof(crsCode)-1);
    if (filter) strncpy(platformFilter, filter, sizeof(platformFilter)-1);
    if (callingCrs) strncpy(callingCrsCode, callingCrs, sizeof(callingCrsCode)-1);
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
    callback = cb;
    WiFiClientSecure httpsClient;
    httpsClient.setInsecure();
    httpsClient.setTimeout(5000);
    httpsClient.setConnectionTimeout(3000);

    int retry = 0;
    while(!httpsClient.connect(wsdlHost, 443) && (retry < 10)){
        delay(100); retry++;
    }
    if(retry >= 10) return UPD_NO_RESPONSE;

    httpsClient.print("GET " + String(wsdlAPI) + " HTTP/1.0\r\nHost: " + String(wsdlHost) + "\r\nConnection: close\r\n\r\n");

    retry = 0;
    while(!httpsClient.available() && retry < 100) { delay(100); retry++; }
    if (retry >= 100) return UPD_TIMEOUT;

    while (httpsClient.connected() || httpsClient.available()) {
        String line = httpsClient.readStringUntil('\n');
        if (line.startsWith("HTTP") && line.indexOf("200 OK") == -1) {
            httpsClient.stop();
            return UPD_HTTP_ERROR;
        }
        if (line == "\r") break;
    }

    loadingWDSL = true;
    xmlStreamingParser parser;
    parser.setListener(this);
    parser.reset();
    grandParentTagName = ""; parentTagName = ""; tagName = ""; tagLevel = 0;

    while(httpsClient.available() || httpsClient.connected()) {
        while (httpsClient.available()) parser.parse(httpsClient.read());
    }
    httpsClient.stop();
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

int nationalRailDataSource::updateData() {
    unsigned long perfTimer = millis();
    bool bChunked = false;
    lastErrorMessage[0] = '\0';

    NationalRailStation xStation;
    stnMessages xMessages;
    memset(&xStation, 0, sizeof(NationalRailStation));
    memset(&xMessages, 0, sizeof(stnMessages));
    
    id = -1; coaches = 0; addedStopLocation = false; keepRoute = false;

    WiFiClientSecure httpsClient;
    httpsClient.setInsecure();
    httpsClient.setTimeout(5000);
    httpsClient.setConnectionTimeout(5000);

    if(!httpsClient.connect(soapHost, 443)) return UPD_NO_RESPONSE;

    int reqRows = NR_MAX_SERVICES;
    if (platformFilter[0]) reqRows = 10;
    
    String data = F("<soap-env:Envelope xmlns:soap-env=\"http://schemas.xmlsoap.org/soap/envelope/\"><soap-env:Header><ns0:AccessToken xmlns:ns0=\"http://thalesgroup.com/RTTI/2013-11-28/Token/types\"><ns0:TokenValue>");
    data += String(nrToken) + F("</ns0:TokenValue></ns0:AccessToken></soap-env:Header><soap-env:Body><ns0:GetDepBoardWithDetailsRequest xmlns:ns0=\"http://thalesgroup.com/RTTI/2021-11-01/ldb/\"><ns0:numRows>") + String(reqRows) + F("</ns0:numRows><ns0:crs>");
    data += String(crsCode) + F("</ns0:crs>");
    if (callingCrsCode[0]) data += "<ns0:filterCrs>" + String(callingCrsCode) + F("</ns0:filterCrs><ns0:filterType>to</ns0:filterType>");
    if (nrTimeOffset) data += "<ns0:timeOffset>" + String(nrTimeOffset) + F("</ns0:timeOffset>");
    data += F("</ns0:GetDepBoardWithDetailsRequest></soap-env:Body></soap-env:Envelope>");

    httpsClient.print("POST " + String(soapAPI) + " HTTP/1.1\r\nHost: " + String(soapHost) + "\r\nContent-Type: text/xml;charset=UTF-8\r\nConnection: close\r\nContent-Length: " + String(data.length()) + "\r\n\r\n" + data);

    if (callback) callback(1, 0);
    
    unsigned long ticker = millis() + 800;
    int retry = 0;
    while(!httpsClient.available() && retry < 30) { delay(100); retry++; }
    if (retry >= 30) return UPD_TIMEOUT;

    while (httpsClient.connected() || httpsClient.available()) {
        String line = httpsClient.readStringUntil('\n');
        if (line.startsWith("HTTP") && line.indexOf("200 OK") == -1) {
            httpsClient.stop();
            return UPD_HTTP_ERROR;
        } else if (line.startsWith("Transfer-Encoding:") && line.indexOf("chunked") >= 0) bChunked = true;
        if (line == "\r") break;
    }

    xmlStreamingParser parser;
    parser.setListener(this);
    parser.reset();
    grandParentTagName = ""; parentTagName = ""; tagName = ""; tagLevel = 0; loadingWDSL = false;
    long bytesRecv = 0;

    // We need to use xStation and xMessages during parsing
    // To do this, we'll temporarily point stationData and messagesData to them
    NationalRailStation oldStation = stationData;
    stnMessages oldMessages = messagesData;
    stationData = xStation;
    messagesData = xMessages;

    while(httpsClient.available() || httpsClient.connected()) {
        while (httpsClient.available()) {
            parser.parse(httpsClient.read());
            bytesRecv++;
            if (millis() > ticker) {
                if (callback) callback(2, stationData.numServices);
                ticker = millis() + 800;
            }
        }
        delay(5);
    }
    httpsClient.stop();

    // After parsing, sanitize and check changes
    sanitiseData();
    
    bool changed = (memcmp(&stationData, &oldStation, sizeof(NationalRailStation)) != 0) || 
                   (memcmp(&messagesData, &oldMessages, sizeof(stnMessages)) != 0);

    if (changed) {
        stationData.boardChanged = true;
    } else {
        stationData = oldStation; // Restore old if no change (keep boardChanged as is)
        messagesData = oldMessages;
    }

    if (callback) callback(3, stationData.numServices);
    snprintf(lastErrorMessage, sizeof(lastErrorMessage), "SUCCESS [%ld] %lums", bytesRecv, (unsigned long)(millis()-perfTimer));
    return changed ? UPD_SUCCESS : UPD_NO_CHANGE;
}

void nationalRailDataSource::sanitiseData() {
    // Basic sanitization from raildataXmlClient
    removeHtmlTags(stationData.location);
    replaceWord(stationData.location, "&amp;", "&");
    for (int i=0; i<stationData.numServices; i++) {
        removeHtmlTags(stationData.service[i].destination);
        replaceWord(stationData.service[i].destination, "&amp;", "&");
        removeHtmlTags(stationData.service[i].calling);
        replaceWord(stationData.service[i].calling, "&amp;", "&");
        removeHtmlTags(stationData.service[i].via);
        replaceWord(stationData.service[i].via, "&amp;", "&");
        removeHtmlTags(stationData.service[i].serviceMessage);
        replaceWord(stationData.service[i].serviceMessage, "&amp;", "&");
        fixFullStop(stationData.service[i].serviceMessage);
    }
    for (int i=0; i<messagesData.numMessages; i++) {
        removeHtmlTags(messagesData.messages[i]);
        replaceWord(messagesData.messages[i], "&amp;", "&");
        pruneFromPhrase(messagesData.messages[i], " More details ");
        fixFullStop(messagesData.messages[i]);
    }
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
    if (x < 0 || x >= stationData.numServices) return;
    for (int i=x; i<stationData.numServices-1; i++) stationData.service[i] = stationData.service[i+1];
    stationData.numServices--;
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
        if (id >= 0 && (strlen(stationData.service[id].calling) + strlen(val) + 10) < NR_MAX_CALLING) {
            if (stationData.service[id].calling[0]) strcat(stationData.service[id].calling, ", ");
            strcat(stationData.service[id].calling, val);
            addedStopLocation = true;
        }
    } else if (tagLevel == 11 && tagPath.endsWith("callingPoint/lt8:st") && addedStopLocation) {
        if (id >= 0 && (strlen(stationData.service[id].calling) + strlen(val) + 4) < NR_MAX_CALLING) {
            strcat(stationData.service[id].calling, " (");
            strcat(stationData.service[id].calling, val);
            strcat(stationData.service[id].calling, ")");
        }
        addedStopLocation = false;
    } else if (tagLevel == 8 && tagName == "lt4:operator") {
        if (id >= 0) strncpy(stationData.service[id].opco, val, 49);
    } else if (tagLevel == 8 && tagName == "lt4:std") {
        if (id < NR_MAX_SERVICES - 1) { id++; stationData.numServices++; }
        strncpy(stationData.service[id].sTime, val, 5);
    } else if (tagLevel == 8 && tagName == "lt4:etd") {
        if (id >= 0) strncpy(stationData.service[id].etd, val, 10);
    } else if (tagLevel == 10 && tagPath.indexOf("destination") != -1 && tagPath.endsWith("locationName")) {
        if (id >= 0) strncpy(stationData.service[id].destination, val, NR_MAX_LOCATION-1);
    } else if (tagLevel == 10 && tagPath.indexOf("destination") != -1 && tagPath.endsWith("via")) {
        if (id >= 0) strncpy(stationData.service[id].via, val, NR_MAX_LOCATION-1);
    } else if (tagLevel == 8 && tagName == "lt4:platform") {
        if (id >= 0) {
            strncpy(stationData.service[id].platform, val, 3);
            if (filterPlatforms && !serviceMatchesFilter(platformFilter, val)) {
                // Technically Darwin client shuffles this, but here we just flag it
            }
        }
    } else if (tagLevel == 6 && tagName == "lt4:locationName") {
        strncpy(stationData.location, val, NR_MAX_LOCATION-1);
    } else if (tagPath.endsWith("nrccMessages/lt:message")) {
        if (messagesData.numMessages < MAXBOARDMESSAGES) {
            strncpy(messagesData.messages[messagesData.numMessages++], val, MAXMESSAGESIZE-1);
        }
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
