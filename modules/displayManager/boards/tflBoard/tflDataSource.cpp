/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/boards/tflBoard/src/tflDataSource.cpp
 * Description: Implementation of TfL data source ported from TfLdataClient.
 */

#include "tflDataSource.hpp"
#include <WiFiClientSecure.h>
#include <Logger.hpp>
#include <algorithm>

// Status codes mapping
#include "../interfaces/iDataSource.hpp"

tflDataSource::tflDataSource() : id(0), maxServicesRead(false), callback(nullptr) {
    memset(&stationData, 0, sizeof(TflStation));
    memset(&messagesData, 0, sizeof(stnMessages));
    lastErrorMsg[0] = '\0';
    tflAppkey[0] = '\0';
    tubeId[0] = '\0';
}

void tflDataSource::configure(const char* naptanId, const char* apiKey, tflDataSourceCallback cb) {
    if (naptanId) strncpy(tubeId, naptanId, sizeof(tubeId)-1);
    if (apiKey) strncpy(tflAppkey, apiKey, sizeof(tflAppkey)-1);
    callback = cb;
}

int tflDataSource::updateData() {
    unsigned long perfTimer = millis();
    long dataReceived = 0;
    bool bChunked = false;
    lastErrorMsg[0] = '\0';

    TflStation xStation;
    stnMessages xMessages;
    memset(&xStation, 0, sizeof(TflStation));
    memset(&xMessages, 0, sizeof(stnMessages));

    JsonStreamingParser parser;
    parser.setListener(this);
    WiFiClientSecure httpsClient;
    httpsClient.setInsecure();
    httpsClient.setTimeout(5000);
    httpsClient.setConnectionTimeout(5000);

    if (!httpsClient.connect(apiHost, 443)) return UPD_NO_RESPONSE;

    String request = "GET /StopPoint/" + String(tubeId) + F("/Arrivals?app_key=") + String(tflAppkey) + F(" HTTP/1.0\r\nHost: ") + String(apiHost) + F("\r\nConnection: close\r\n\r\n");
    httpsClient.print(request);
    if (callback) callback();

    unsigned long ticker = millis() + 800;
    int retry = 0;
    while(!httpsClient.available() && retry < 40) { delay(200); retry++; }
    if (retry >= 40) return UPD_TIMEOUT;

    String statusLine = httpsClient.readStringUntil('\n');
    if (!statusLine.startsWith(F("HTTP/")) || statusLine.indexOf(F("200 OK")) == -1) {
        httpsClient.stop();
        return UPD_HTTP_ERROR;
    }

    while (httpsClient.connected() || httpsClient.available()) {
        String line = httpsClient.readStringUntil('\n');
        if (line == F("\r")) break;
        if (line.startsWith(F("Transfer-Encoding:")) && line.indexOf(F("chunked")) >= 0) bChunked = true;
    }

    // Temporarily point to locals for parsing
    TflStation oldStation = stationData;
    stnMessages oldMessages = messagesData;
    stationData = xStation;
    messagesData = xMessages;
    
    id = 0; maxServicesRead = false;
    while((httpsClient.available() || httpsClient.connected()) && !maxServicesRead) {
        while(httpsClient.available() && !maxServicesRead) {
            parser.parse(httpsClient.read());
            dataReceived++;
            if (millis() > ticker) { if (callback) callback(); ticker = millis() + 800; }
        }
        delay(10);
    }
    httpsClient.stop();

    // Fetch Disruption Msgs
    if (httpsClient.connect(apiHost, 443)) {
        request = "GET /StopPoint/" + String(tubeId) + F("/Disruption?getFamily=true&flattenResponse=true&app_key=") + String(tflAppkey) + F(" HTTP/1.0\r\nHost: ") + String(apiHost) + F("\r\nConnection: close\r\n\r\n");
        httpsClient.print(request);
        retry = 0;
        while(!httpsClient.available() && retry < 40) { delay(200); retry++; }
        if (httpsClient.available()) {
            statusLine = httpsClient.readStringUntil('\n');
            if (statusLine.indexOf(F("200 OK")) != -1) {
                while (httpsClient.connected() || httpsClient.available()) { if (httpsClient.readStringUntil('\n') == F("\r")) break; }
                parser.reset(); id = 0; maxServicesRead = false;
                while((httpsClient.available() || httpsClient.connected()) && !maxServicesRead) {
                    while(httpsClient.available() && !maxServicesRead) {
                        parser.parse(httpsClient.read());
                        dataReceived++;
                    }
                    delay(10);
                }
            }
        }
    }
    httpsClient.stop();

    // Sort and Sanitize
    std::sort(stationData.service, stationData.service + stationData.numServices, compareTimes);
    if (stationData.numServices > TFL_MAX_SERVICES) stationData.numServices = TFL_MAX_SERVICES;

    for (int i=0; i<messagesData.numMessages; i++) replaceWord(messagesData.messages[i], "\\n", "");

    // Populate expectedTime strings
    for (int i=0; i<stationData.numServices; i++) {
        int m = stationData.service[i].timeToStation / 60;
        if (m == 0) strcpy(stationData.service[i].expectedTime, "Due");
        else sprintf(stationData.service[i].expectedTime, "%d mins", m);
    }

    bool changed = (memcmp(&stationData, &oldStation, sizeof(TflStation)) != 0) || 
                   (memcmp(&messagesData, &oldMessages, sizeof(stnMessages)) != 0);

    if (changed) stationData.boardChanged = true;
    else { stationData = oldStation; messagesData = oldMessages; }

    snprintf(lastErrorMsg, sizeof(lastErrorMsg), "SUCCESS %lums [%ld]", (unsigned long)(millis()-perfTimer), dataReceived);
    return changed ? UPD_SUCCESS : UPD_NO_CHANGE;
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
    if (currentKey == "id") {
        if (stationData.numServices < TFL_MAX_FETCH) id = stationData.numServices++;
        else maxServicesRead = true;
    } else if (currentKey == "description") {
        if (messagesData.numMessages < MAXBOARDMESSAGES) id = messagesData.numMessages++;
        else maxServicesRead = true;
    }
}
void tflDataSource::value(String val) {
    if (currentKey == "destinationName") {
        if (val.endsWith(" Underground Station")) val = val.substring(0, val.length()-20);
        else if (val.endsWith(" DLR Station")) val = val.substring(0, val.length()-12);
        strncpy(stationData.service[id].destination, val.c_str(), TFL_MAX_LOCATION-1);
    } else if (currentKey == "timeToStation") stationData.service[id].timeToStation = val.toInt();
    else if (currentKey == "lineName") strncpy(stationData.service[id].lineName, val.c_str(), TFL_MAX_LINE-1);
    else if (currentKey == "description") strncpy(messagesData.messages[id], val.c_str(), MAXMESSAGESIZE-1);
}
void tflDataSource::endArray() {}
void tflDataSource::endObject() {}
void tflDataSource::endDocument() {}
void tflDataSource::startArray() {}
void tflDataSource::startObject() {}
