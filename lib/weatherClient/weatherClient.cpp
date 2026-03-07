/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * OpenWeatherMap Weather Client Library
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/weatherClient/weatherClient.cpp
 * Description: Exported functions and classes.
 *
 * Exported Functions/Classes:
 * - updateWeather: Update weather
 * - whitespace: Whitespace
 * - startDocument: Start document
 * - key: Key
 * - value: Value
 * - endArray: End array
 * - endObject: End object
 * - endDocument: End document
 * - startArray: Start array
 * - startObject: Start object
 */

#include <weatherClient.h>
#include <JsonListener.h>
#include <WiFiClient.h>

weatherClient::weatherClient() {}

/**
 * @brief Update weather
 * @param apiKey
 * @param lat
 * @param lon
 * @return Return value
 */
bool weatherClient::updateWeather(String apiKey, String lat, String lon) {

    unsigned long perfTimer = millis();
    lastErrorMsg = "";

    JsonStreamingParser parser;
    parser.setListener(this);
    WiFiClient httpClient;

    int retryCounter=0;
    while (!httpClient.connect(apiHost, 80) && (retryCounter++ < 15)){
        delay(200);
    }
    if (retryCounter>=15) {
        lastErrorMsg += F("Connection timeout");
        return false;
    }

    String request = "GET /data/2.5/weather?units=metric&lang=en&lat=" + lat + F("&lon=") + lon + F("&appid=") + apiKey + F(" HTTP/1.0\r\nHost: ") + String(apiHost) + F("\r\nConnection: close\r\n\r\n");
    httpClient.print(request);
    retryCounter=0;
    while(!httpClient.available() && retryCounter++ < 40) {
        delay(200);
    }

    if (!httpClient.available()) {
        // no response within 8 seconds so exit
        httpClient.stop();
        lastErrorMsg += F("Response timeout (GET)");
        return false;
    }

    // Parse status code
    String statusLine = httpClient.readStringUntil('\n');
    if (!statusLine.startsWith(F("HTTP/")) || statusLine.indexOf(F("200 OK")) == -1) {
        httpClient.stop();

        if (statusLine.indexOf(F("401")) > 0) {
            lastErrorMsg = F("Not Authorized");
        } else if (statusLine.indexOf(F("500")) > 0) {
            lastErrorMsg = F("Server Error");
        } else {
            lastErrorMsg = statusLine;
        }
        return false;
    }

    // Skip the remaining headers
    while (httpClient.connected() || httpClient.available()) {
        String line = httpClient.readStringUntil('\n');
        if (line == "\r") break;
    }

    bool isBody = false;
    char c;
    weatherItem=0;

    unsigned long dataSendTimeout = millis() + 10000UL;
    while((httpClient.available() || httpClient.connected()) && (millis() < dataSendTimeout)) {
        while(httpClient.available()) {
            c = httpClient.read();
            if (c == '{' || c == '[') isBody = true;
            if (isBody) parser.parse(c);
        }
        delay(5);
    }
    httpClient.stop();
    if (millis() >= dataSendTimeout) {
        lastErrorMsg += F("Data timeout");
        return false;
    }

    lastErrorMsg="Success - took " + String(millis()-perfTimer) + F("ms");

    currentWeather = description + " " + String((int)round(temperature)) + F("\xB0 Wind: ") + String((int)round(windSpeed)) + F("mph");
    return true;
}

/**
 * @brief Whitespace
 * @param c
 */
void weatherClient::whitespace(char c) {}

/**
 * @brief Start document
 */
void weatherClient::startDocument() {}

/**
 * @brief Key
 * @param key
 */
void weatherClient::key(String key) {
    currentKey = key;
}

/**
 * @brief Value
 * @param value
 */
void weatherClient::value(String value) {
    if (currentObject == F("weather") && weatherItem==0) {
        // Only read the first weather entry in the array
        if (currentKey == F("description")) description = value;
    }
    else if (currentKey == F("temp")) temperature = value.toFloat();
    // Windspeed reported in mps, converting to mph
    else if (currentKey == F("speed")) windSpeed = value.toFloat() * 2.23694;
}

/**
 * @brief End array
 */
void weatherClient::endArray() {}

/**
 * @brief End object
 */
void weatherClient::endObject() {
    if (currentObject == F("weather")) weatherItem++;
    currentObject = "";
}

/**
 * @brief End document
 */
void weatherClient::endDocument() {}

/**
 * @brief Start array
 */
void weatherClient::startArray() {}

/**
 * @brief Start object
 */
void weatherClient::startObject() {
    currentObject = currentKey;
}
