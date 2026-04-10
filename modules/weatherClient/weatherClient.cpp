/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * OpenWeatherMap Weather Client Library
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/weatherClient/weatherClient.cpp
 * Description: Implementation of the OpenWeatherMap JSON client using streaming parser.
 *
 * Exported Functions/Classes:
 * - weatherClient: [Class implementation]
 *   - updateWeather: Strategic UI entry point for weather sync.
 *   - executeFetch: Background thread implementation of HTTP/JSON sequence.
 */

#include <weatherClient.hpp>
#include <JsonListener.h>
#include <NetworkClientSecure.h>
#include <logger.hpp>
#include <memory>
#include <appContext.hpp>

extern class appContext appContext;

/**
 * @brief Implements the iConfigurable interface.
 */
void weatherClient::reapplyConfig(const Config& config) {
    bool hasOwmKey = false;
    activeApiKey = "";

    // First try the specifically chosen weatherKeyId
    for (int i = 0; i < config.keyCount; i++) {
        if (strlen(config.weatherKeyId) > 0 && strcmp(config.keys[i].id, config.weatherKeyId) == 0) {
            activeApiKey = String(config.keys[i].token);
            hasOwmKey = true;
            break;
        }
    }
    
    // Fallback to first OWM key if designated key isn't found
    if (!hasOwmKey) {
        for (int i = 0; i < config.keyCount; i++) {
            if (strcmp(config.keys[i].type, "owm") == 0) {
                activeApiKey = String(config.keys[i].token);
                hasOwmKey = true;
                break;
            }
        }
    }

    setWeatherEnabled(hasOwmKey);
    
    if (hasOwmKey) {
        bool found = false;
        MessagePool& pool = appContext.getGlobalMessagePool();
        const char* attr = getAttributionString();
        for (size_t i = 0; i < pool.getCount(); i++) {
            const char* msg = pool.getMessage(i);
            if (msg && strcmp(msg, attr) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            pool.addMessage(attr);
        }
    }
}

/**
 * @brief Default constructor for weatherClient.
 */
weatherClient::weatherClient() {
    weatherMutex = xSemaphoreCreateMutex();
}

/**
 * @brief Strategic UI entry point for weather synchronization.
 * Checks for duplicate requests and existing data freshness before queuing a background task.
 * @param status Reference to the WeatherStatus object (Double Buffer destination).
 * @param apiKeyId Pointer to the specific API key reference from config.
 * @param overrideToken Optional raw override for testing/validation bypass.
 * @return true if the fetch was successfully queued or found to be redundant.
 */
bool weatherClient::updateWeather(WeatherStatus& status, const char* apiKeyId, const char* overrideToken) {
    if (fetchPending) {
        LOG_INFO("DATA", "Weather Client: Fetch already pending in Worker Queue");
        return true; 
    }

    // Check if we already have valid data for this exact location that is still fresh
    if (status.isValid() && 
        status.lat == bgStatus.lat && 
        status.lon == bgStatus.lon && 
        activeStatus == &status &&
        millis() < nextWeatherUpdate) {
        return true;
    }

    String apiKey = "";
    if (overrideToken && strlen(overrideToken) > 0) {
        apiKey = String(overrideToken);
    } else if (apiKeyId && strlen(apiKeyId) > 0) {
        ApiKey* key = appContext.getConfigManager().getKeyById(apiKeyId);
        if (key && strlen(key->token) > 0) {
            apiKey = String(key->token);
        }
    }

    if (apiKey.length() == 0) {
        const Config& cfg = appContext.getConfigManager().getConfig();
        for (int i = 0; i < cfg.keyCount; i++) {
            if (strcmp(cfg.keys[i].type, "owm") == 0) {
                apiKey = String(cfg.keys[i].token);
                break;
            }
        }
    }

    if (apiKey.length() == 0) {
        strcpy(lastErrorMsg, "No valid API Key");
        LOG_WARN("DATA", lastErrorMsg);
        status.status = WeatherUpdateStatus::NOT_CONFIGURED;
        return false;
    }

    activeApiKey = apiKey;
    activeStatus = &status;
    bgStatus.lat = status.lat;
    bgStatus.lon = status.lon;
    bgStatus.status = WeatherUpdateStatus::NO_DATA;
    parsingComplete = false;

    LOG_INFO("DATA", "Weather Client: Requesting priority fetch from DataManager");
    fetchPending = true;
    appContext.getDataManager().requestPriorityFetch(this);
    return true;
}

/**
 * @brief Core synchronous implementation of the HTTP and JSON parsing pipeline.
 * Designed to be executed within a dedicated background task.
 * Manages sockets, error handling, and memory-safe struct synchronization.
 */
void weatherClient::executeFetch() {
    if (activeApiKey.length() == 0) {
        setNextFetchTime(millis() + 600000);
        return;
    }
    unsigned long perfTimer = millis();
    strcpy(lastErrorMsg, "");
    
    char latBuf[16], lonBuf[16];
    dtostrf(bgStatus.lat, 1, 4, latBuf);
    dtostrf(bgStatus.lon, 1, 4, lonBuf);

    auto parser = std::make_unique<JsonStreamingParser>();
    auto httpClient = std::make_unique<NetworkClientSecure>();
    httpClient->setInsecure();

    #define WRAP_UP_ERROR() { \
        setNextFetchTime(millis() + (1000 * 60)); \
        xSemaphoreTake(weatherMutex, portMAX_DELAY); \
        if (activeStatus) *activeStatus = bgStatus; \
        xSemaphoreGive(weatherMutex); \
        fetchPending = false; \
        return; \
    }

    if (!parser || !httpClient) {
        LOG_ERROR("DATA", "Weather Client: Memory allocation failed!");
        bgStatus.status = WeatherUpdateStatus::DATA_ERROR;
        WRAP_UP_ERROR();
    }

    parser->setListener(this);
    
    // --- Step 1: Protocol Handshake ---
    int retryCounter=0;
    while (!httpClient->connect(apiHost, 443) && (retryCounter++ < 15)){
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    if (retryCounter>=15) {
        strcpy(lastErrorMsg, "Connection timeout");
        LOG_WARN("DATA", lastErrorMsg);
        bgStatus.status = WeatherUpdateStatus::DATA_ERROR;
        WRAP_UP_ERROR();
    }

    // --- Step 2: GET Request ---
    {
        char request[512];
        int len = snprintf(request, sizeof(request), 
                 "GET /data/2.5/weather?units=metric&lang=en&lat=%s&lon=%s&appid=%s HTTP/1.1\r\n"
                 "Host: %s\r\n"
                 "User-Agent: ESP32-Departures-Board\r\n"
                 "Connection: close\r\n\r\n",
                 latBuf, lonBuf, activeApiKey.c_str(), apiHost);

        if (len >= (int)sizeof(request)) {
            LOG_ERRORf("DATA", "Weather Client: Request URL too long! (%d bytes)", len);
            bgStatus.status = WeatherUpdateStatus::DATA_ERROR;
            WRAP_UP_ERROR();
        }

        LOG_DEBUGf("DATA", "Weather Client: Requesting URL (Length: %d)", len); 
        httpClient->print(request);
    }
    retryCounter=0;
    while(!httpClient->available() && retryCounter++ < 40) {
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    if (!httpClient->available()) {
        // no response within 8 seconds so exit
        httpClient->stop();
        strcpy(lastErrorMsg, "Response timeout (GET)");
        LOG_WARN("DATA", lastErrorMsg);
        bgStatus.status = WeatherUpdateStatus::DATA_ERROR;
        WRAP_UP_ERROR();
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
        bgStatus.status = WeatherUpdateStatus::DATA_ERROR;
        WRAP_UP_ERROR();
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
    int parseYield = 0;
    while((httpClient->available() || httpClient->connected()) && (millis() < dataSendTimeout) && !parsingComplete) {
        while(httpClient->available() && !parsingComplete) {
            c = httpClient->read();
            #ifdef ENABLE_DEBUG_LOG
            // Serial.print(c); // Raw dump blocked to avoid thread collision
            #endif
            if (c == '{' || c == '[') isBody = true;
            if (isBody) parser->parse(c);
            
            // --- Arcane Logic ---
            // On single-core ESP32 variants (e.g. ESP32-C3), the Wi-Fi stack and user application
            // share the exact same processor core tightly via the RTOS scheduler. By explicitly
            // yielding execution context via vTaskDelay(1) every 500 byte blocks, we guarantee
            // network hardware interrupts service without triggering Task Watchdog Timers (TWDT).
            parseYield++;
            if (parseYield % 500 == 0) vTaskDelay(1);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    #ifdef ENABLE_DEBUG_LOG
    Serial.println(); // Newline after dump
    #endif
    httpClient->stop();
    if (millis() >= dataSendTimeout) {
        strcpy(lastErrorMsg, "Data timeout");
        LOG_WARN("DATA", lastErrorMsg);
        bgStatus.status = WeatherUpdateStatus::DATA_ERROR;
        WRAP_UP_ERROR();
    }

    snprintf(lastErrorMsg, sizeof(lastErrorMsg), "Success - took %lums", static_cast<unsigned long>(millis()-perfTimer));
    bgStatus.status = WeatherUpdateStatus::READY;
    setNextFetchTime(millis() + (1000 * 60 * 15)); // Update every 15 mins on success
    
    xSemaphoreTake(weatherMutex, portMAX_DELAY);
    if (activeStatus) {
        *activeStatus = bgStatus;
    }
    xSemaphoreGive(weatherMutex);
    
#ifdef ENABLE_DEBUG_LOG
    UBaseType_t hwm = uxTaskGetStackHighWaterMark(NULL);
    LOG_DEBUGf("DATA", "Weather Task Stack High Water Mark: %d words", (int)hwm);
#endif
    fetchPending = false;
    return;
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
        if (currentKey == F("description")) {
            strlcpy(bgStatus.description, value.c_str(), sizeof(bgStatus.description));
            LOG_DEBUGf("DATA", "Weather Parser: Description=%s", value.c_str());
        }
        else if (currentKey == F("id")) {
            bgStatus.conditionId = value.toInt();
            LOG_DEBUG("DATA", "Weather Parser: ID=" + value);
        }
        else if (currentKey == F("icon")) {
            // OWM icon codes like "01d" or "01n". If ends with 'n', it's night.
            if (value.length() >= 3) {
                bgStatus.isNight = (value.charAt(2) == 'n');
                LOG_DEBUGf("DATA", "Weather Parser: Icon=%s (isNight=%s)", value.c_str(), bgStatus.isNight ? "true" : "false");
            }
        }
    }
    else if (currentKey == F("temp")) {
        bgStatus.temp = value.toFloat();
        LOG_DEBUG("DATA", "Weather Parser: Temp=" + value);
    }
    // Windspeed reported in mps, converting to knots (1 m/s ≈ 1.94384 knots)
    else if (currentKey == F("speed")) {
        bgStatus.windSpeed = value.toFloat() * 1.94384;
        LOG_DEBUGf("DATA", "Weather Parser: Wind=%s (%.2f knots)", value.c_str(), bgStatus.windSpeed);
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
void weatherClient::endDocument() {
    parsingComplete = true;
}

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

void weatherClient::serializeData(JsonObject& doc) {
    if (weatherMutex && xSemaphoreTake(weatherMutex, pdMS_TO_TICKS(50)) == pdPASS) {
        doc["temp"] = bgStatus.temp;
        doc["condition"] = bgStatus.description;
        doc["wind_knots"] = bgStatus.windSpeed;
        doc["is_night"] = bgStatus.isNight;
        xSemaphoreGive(weatherMutex);
    }
}
