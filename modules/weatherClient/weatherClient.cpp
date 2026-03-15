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
 * Module: modules/weatherClient/weatherClient.cpp
 * Description: Implementation of the OpenWeatherMap JSON client.
 *
 * Exported Functions/Classes:
 * - weatherClient::weatherClient: Constructor.
 * - weatherClient::updateWeather: Connects to OpenWeatherMap API and parses JSON.
 * - weatherClient::reapplyConfig: Provisions setting from global configuration.
 */

#include <weatherClient.h>
#include <JsonListener.h>
#include <WiFiClient.h>
#include <logger.hpp>
#include <memory>

/**
 * @brief Implements the iConfigurable interface.
 */
void weatherClient::reapplyConfig(const Config& config) {
    setOpenWeatherMapApiKey(config.owmToken);
    setWeatherEnabled(config.owmToken[0] != '\0');
}

weatherClient::weatherClient() {}

/**
 * @brief Connects to OpenWeatherMap API, retrieves the current weather for a location, and parses the JSON response.
 * @param status Reference to the WeatherStatus object to update (must have valid lat/lon).
 * @return True if the metadata was successfully fetched and parsed, otherwise false.
 */
bool weatherClient::updateWeather(WeatherStatus& status) {
    activeStatus = &status;
    activeStatus->status = WeatherUpdateStatus::DATA_ERROR; // Assume error until success

    String apiKey = String(openWeatherMapApiKey);
    char latBuf[16], lonBuf[16];
    dtostrf(status.lat, 4, 4, latBuf);
    dtostrf(status.lon, 4, 4, lonBuf);
    String lat = String(latBuf);
    String lon = String(lonBuf);

    unsigned long perfTimer = millis();
    strcpy(lastErrorMsg, "");

    std::unique_ptr<JsonStreamingParser> parser(new (std::nothrow) JsonStreamingParser());
    std::unique_ptr<WiFiClient> httpClient(new (std::nothrow) WiFiClient());

    if (!parser || !httpClient) {
        LOG_ERROR("DATA", "Weather Client: Memory allocation failed!");
        return false;
    }

    parser->setListener(this);
    
    // --- Step 1: Protocol Handshake ---
    int retryCounter=0;
    while (!httpClient->connect(apiHost, 80) && (retryCounter++ < 15)){
        if (yieldCallback) yieldCallback();
        delay(200);
    }
    if (retryCounter>=15) {
        strcpy(lastErrorMsg, "Connection timeout");
        LOG_WARN("DATA", lastErrorMsg);
        return false;
    }

    // --- Step 2: GET Request ---
    String request = "GET /data/2.5/weather?units=metric&lang=en&lat=" + lat + F("&lon=") + lon + F("&appid=") + apiKey + F(" HTTP/1.0\r\nHost: ") + String(apiHost) + F("\r\nConnection: close\r\n\r\n");
    LOG_DEBUG("DATA", "Weather Client: Requesting " + request); // API Key will be redacted by Logger
    httpClient->print(request);
    retryCounter=0;
    while(!httpClient->available() && retryCounter++ < 40) {
        if (yieldCallback) yieldCallback();
        delay(200);
    }

    if (!httpClient->available()) {
        // no response within 8 seconds so exit
        httpClient->stop();
        strcpy(lastErrorMsg, "Response timeout (GET)");
        LOG_WARN("DATA", lastErrorMsg);
        return false;
    }

    // --- Step 3: Status Check ---
    // Parse status code
    String statusLine = httpClient->readStringUntil('\n');
    if (!statusLine.startsWith(F("HTTP/")) || statusLine.indexOf(F("200 OK")) == -1) {
        httpClient->stop();

        if (statusLine.indexOf(F("401")) > 0) {
            strcpy(lastErrorMsg, "Not Authorized");
            LOG_ERROR("DATA", lastErrorMsg);
        } else if (statusLine.indexOf(F("500")) > 0) {
            strcpy(lastErrorMsg, "Server Error");
            LOG_WARN("DATA", lastErrorMsg);
        } else {
            strncpy(lastErrorMsg, statusLine.c_str(), sizeof(lastErrorMsg)-1);
            lastErrorMsg[sizeof(lastErrorMsg)-1] = '\0';
            LOG_WARN("DATA", lastErrorMsg);
        }
        return false;
    }

    // --- Step 4: Skip Headers ---
    // Skip the remaining headers
    while (httpClient->connected() || httpClient->available()) {
        String line = httpClient->readStringUntil('\n');
        if (line == "\r") break;
    }

    bool isBody = false;
    char c;
    weatherItem=0;

    unsigned long dataSendTimeout = millis() + 10000UL;
    LOG_DEBUG("DATA", "Weather Client: Receiving streaming response...");
    
    // --- Step 5: Streaming Parse ---
    while((httpClient->available() || httpClient->connected()) && (millis() < dataSendTimeout)) {
        while(httpClient->available()) {
            c = httpClient->read();
            #ifdef ENABLE_DEBUG_LOG
            Serial.print(c); // Raw dump to serial (Logger doesn't handle char-by-char)
            #endif
            if (c == '{' || c == '[') isBody = true;
            if (isBody) parser->parse(c);
        }
        if (yieldCallback) yieldCallback();
        delay(5);
    }
    #ifdef ENABLE_DEBUG_LOG
    Serial.println(); // Newline after dump
    #endif
    httpClient->stop();
    if (millis() >= dataSendTimeout) {
        strcpy(lastErrorMsg, "Data timeout");
        LOG_WARN("DATA", lastErrorMsg);
        return false;
    }

    snprintf(lastErrorMsg, sizeof(lastErrorMsg), "Success - took %lums", static_cast<unsigned long>(millis()-perfTimer));
    activeStatus->status = WeatherUpdateStatus::READY;

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
    if (!activeStatus) return;

    if (currentObject == F("weather") && weatherItem==0) {
        // Only read the first weather entry in the array
        if (currentKey == F("description")) {
            strlcpy(activeStatus->description, value.c_str(), sizeof(activeStatus->description));
            LOG_DEBUG("DATA", "Weather Parser: Description=" + value);
        }
        else if (currentKey == F("id")) {
            activeStatus->conditionId = value.toInt();
            LOG_DEBUG("DATA", "Weather Parser: ID=" + value);
        }
        else if (currentKey == F("icon")) {
            // OWM icon codes like "01d" or "01n". If ends with 'n', it's night.
            if (value.length() >= 3) {
                activeStatus->isNight = (value.charAt(2) == 'n');
                LOG_DEBUG("DATA", "Weather Parser: Icon=" + value + " (isNight=" + String(activeStatus->isNight ? "true" : "false") + ")");
            }
        }
    }
    else if (currentKey == F("temp")) {
        activeStatus->temp = value.toFloat();
        LOG_DEBUG("DATA", "Weather Parser: Temp=" + value);
    }
    // Windspeed reported in mps, converting to knots (1 m/s ≈ 1.94384 knots)
    else if (currentKey == F("speed")) {
        activeStatus->windSpeed = value.toFloat() * 1.94384;
        LOG_DEBUG("DATA", "Weather Parser: Wind=" + value + " (" + String(activeStatus->windSpeed) + " knots)");
    }
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
