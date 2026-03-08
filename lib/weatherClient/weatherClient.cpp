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
 * Description: Implementation of the OpenWeatherMap JSON client.
 *
 * Exported Functions/Classes:
 * - weatherClient::updateWeather: Connects to API and updates current weather properties.
 * - weatherClient::whitespace: JSON whitespace handler.
 * - weatherClient::startDocument: JSON handler triggered at start of document.
 * - weatherClient::key: JSON handler triggered for each object key.
 * - weatherClient::value: JSON handler triggered for each key value.
 * - weatherClient::endArray: JSON handler triggered when exiting an array.
 * - weatherClient::endObject: JSON handler triggered when exiting an object.
 * - weatherClient::endDocument: JSON handler triggered at end of document.
 * - weatherClient::startArray: JSON handler triggered when entering an array.
 * - weatherClient::startObject: JSON handler triggered when entering an object.
 */

#include <weatherClient.h>
#include <JsonListener.h>
#include <WiFiClient.h>

weatherClient::weatherClient() {}

/**
 * @brief Connects to OpenWeatherMap API, retrieves the current weather for a location, and parses the JSON response.
 * @param apiKey The user's OpenWeatherMap API key.
 * @param lat The latitude of the location.
 * @param lon The longitude of the location.
 * @return True if the metadata was successfully fetched and parsed, otherwise false.
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
 * @brief JSON whitespace handler.
 * @param c The whitespace char.
 */
void weatherClient::whitespace(char c) {}

/**
 * @brief JSON handler triggered at start of document.
 */
void weatherClient::startDocument() {}

/**
 * @brief JSON handler triggered for each object key.
 * @param key The string name of the parsed key.
 */
void weatherClient::key(String key) {
    currentKey = key;
}

/**
 * @brief JSON handler triggered for each key value.
 * @param value The scalar string value.
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
 * @brief JSON handler triggered when exiting an array.
 */
void weatherClient::endArray() {}

/**
 * @brief JSON handler triggered when exiting an object.
 */
void weatherClient::endObject() {
    if (currentObject == F("weather")) weatherItem++;
    currentObject = "";
}

/**
 * @brief JSON handler triggered at end of document.
 */
void weatherClient::endDocument() {}

/**
 * @brief JSON handler triggered when entering an array.
 */
void weatherClient::startArray() {}

/**
 * @brief JSON handler triggered when entering an object.
 */
void weatherClient::startObject() {
    currentObject = currentKey;
}
