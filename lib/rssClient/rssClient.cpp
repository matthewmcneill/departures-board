/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * rssClient Library
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/rssClient/rssClient.cpp
 * Description: Implementation of the RSS XML client using a streaming parser.
 *
 * Exported Functions/Classes:
 * - rssClient: [Class implementation]
 *   - loadFeed: Human-driven manual fetch for specific URLs.
 *   - updateData: Background-driven schedule update.
 *   - executeFetch: Internal FreeRTOS Task worker for network I/O.
 *   - addRssMessage: MessagePool integration for board rendering.
 */

#include <rssClient.hpp>
#include <xmlListener.hpp>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <logger.hpp>
#include <memory>
#include <appContext.hpp>
#include <messaging/messagePool.hpp>

extern class appContext appContext;

/**
 * @brief Implements the iConfigurable interface.
 */
void rssClient::reapplyConfig(const Config& config) {
    setRssURL(config.rssUrl);
    setRssName(config.rssName);
    setRssEnabled(config.rssEnabled);
    LOG_INFOf("CONFIG", "RSS Config: Enabled=%s, URL=%s", config.rssEnabled ? "true" : "false", config.rssUrl);
}

rssClient::rssClient() {
    rssMutex = xSemaphoreCreateMutex();
}

/**
 * @brief Trims leading and trailing whitespace from a character array in-place.
 * @param str The character array to trim.
 */
void rssClient::trim(char* str) {
    char* start = str;
    while (*start && isspace(static_cast<unsigned char>(*start)))
        ++start;

    char* end = start + strlen(start);
    while (end > start && isspace(static_cast<unsigned char>(*(end - 1))))
        --end;

    *end = '\0';

    if (start != str)
        memmove(str, start, end - start + 1);
}

/**
 * @brief Manual entry point to fetch a specific RSS feed.
 * Overrides the internal activeUrl and triggers a priority fetch.
 * @param url The target RSS/XML URL.
 * @return Latest UpdateStatus (typically PENDING if queued).
 */
UpdateStatus rssClient::loadFeed(String url) {
    activeUrl = url;
    return updateData();
}

UpdateStatus rssClient::updateData() {
    if (activeUrl.length() == 0 && strlen(rssURL) > 0) activeUrl = String(rssURL);
    
    if (activeUrl.length() == 0) {
        LOG_WARN("DATA", "RSS Client: No URL configured. Skipping fetch.");
        lastRssUpdateResult = UpdateStatus::SUCCESS; // (quiet skip)
        return lastRssUpdateResult;
    }

    LOG_INFO("DATA", "RSS Client: Requesting priority fetch from DataManager");
    lastRssUpdateResult = UpdateStatus::PENDING;
    appContext.getDataManager().requestPriorityFetch(this);
    return UpdateStatus::PENDING;
}

UpdateStatus rssClient::testConnection(const char* token, const char* stationId) {
    if (stationId && strlen(stationId) > 0) activeUrl = String(stationId);
    else if (strlen(rssURL) > 0) activeUrl = String(rssURL);
    
    executeFetch();
    return (lastRssUpdateResult == UpdateStatus::NO_CHANGE) ? UpdateStatus::SUCCESS : lastRssUpdateResult;
}

/**
 * @brief Synchronous implementation of the HTTP and XML parsing sequence.
 * Designed to be executed within a pinned FreeRTOS background task.
 * Manages secure clients, redirects, and thread-safe headline storage.
 */
void rssClient::executeFetch() {
    if (activeUrl.length() == 0) {
        setNextFetchTime(millis() + 600000);
        return;
    }
    String url = activeUrl;
    std::unique_ptr<HTTPClient> http(new (std::nothrow) HTTPClient());
    std::unique_ptr<WiFiClient> client(new (std::nothrow) WiFiClient());
    std::unique_ptr<WiFiClientSecure> clientSecure(new (std::nothrow) WiFiClientSecure());

    if (!http || !client || !clientSecure) {
        LOG_ERROR("DATA", "RSS Client: Memory allocation failed for clients!");
        lastRssUpdateResult = UpdateStatus::DATA_ERROR;
        setNextFetchTime(millis() + 30000);
        return;
    }

    unsigned long perfTimer = millis();
    int redirectCount = 0;
    const int maxRedirects = 5;

    // --- Step 1: Initialization ---
    strcpy(lastErrorMessage, "Success");
    clientSecure->setInsecure();
    http->setReuse(false);
    bgNumRssTitles = 0;
    LOG_INFOf("DATA", "RSS Client: Loading feed from %s", url.c_str());

    while (redirectCount < maxRedirects) {
        // --- Step 2: HTTP Connection ---
        if (url.startsWith(F("https"))) http->begin(*clientSecure, url);
        else http->begin(*client, url);
        int httpCode = http->GET();
        if (httpCode == HTTP_CODE_OK) {
            WiFiClient *stream = http->getStreamPtr();
            std::unique_ptr<xmlStreamingParser> parser(new (std::nothrow) xmlStreamingParser());
            if (!parser) {
                LOG_ERROR("DATA", "RSS Client: Memory allocation failed for parser!");
                lastRssUpdateResult = UpdateStatus::DATA_ERROR;
                setNextFetchTime(millis() + 30000);
                return;
            }
            parser->setListener(this);
            parser->reset();
            tagPath[0] = '\0';
            tagLevel = 0;
            long dataReceived = 0;
            char c;
            unsigned long dataSendTimeout = millis() + 10000UL;

            int parseYield = 0;
            // --- Step 3: Streaming Parse ---
            while((stream->available() || http->connected()) && millis() < dataSendTimeout && bgNumRssTitles < MAX_RSS_TITLES) {
                while (stream->available() && bgNumRssTitles < MAX_RSS_TITLES) {
                    c = stream->read();
                    parser->parse(c);
                    dataReceived++;
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

            http->end();
            if (millis() >= dataSendTimeout && bgNumRssTitles < MAX_RSS_TITLES) {
                snprintf(lastErrorMessage, sizeof(lastErrorMessage), "Timed out during data receive operation - %ld bytes received", dataReceived);
                LOG_WARN("DATA", lastErrorMessage);
                lastRssUpdateResult = UpdateStatus::TIMEOUT;
                setNextFetchTime(millis() + 30000);
                return;
            }
            snprintf(lastErrorMessage, sizeof(lastErrorMessage), "Success: %ld bytes took %lums with %d redirects", dataReceived, static_cast<unsigned long>(millis()-perfTimer), redirectCount);
            LOG_INFOf("DATA", "RSS Feed: Successfully fetched %ld bytes. Found %d titles.", dataReceived, bgNumRssTitles);
            
            xSemaphoreTake(rssMutex, portMAX_DELAY);
            numRssTitles = bgNumRssTitles;
            for (int i=0; i<bgNumRssTitles; i++) {
                strlcpy(rssTitle[i], bgRssTitle[i], MAX_RSS_TITLE_SIZE);
            }
            lastRssUpdateResult = UpdateStatus::SUCCESS;
            setNextFetchTime(millis() + 600000); // 10 minutes on success
            xSemaphoreGive(rssMutex);
#ifdef ENABLE_DEBUG_LOG
            UBaseType_t hwm = uxTaskGetStackHighWaterMark(NULL);
            LOG_DEBUGf("DATA", "RSS Task Stack High Water Mark: %d words", (int)hwm);
#endif
            return;
            break;
        } else if (httpCode == HTTP_CODE_MOVED_PERMANENTLY ||
                   httpCode == HTTP_CODE_FOUND ||
                   httpCode == HTTP_CODE_TEMPORARY_REDIRECT ||
                   httpCode == HTTP_CODE_PERMANENT_REDIRECT) {
            // Handle redirect
            String newUrl = http->getLocation();
            http->end();  // End current request before retrying
            if (newUrl.length() == 0) {
                strcpy(lastErrorMessage, "HTTP Redirect without Location header!");
                LOG_WARN("DATA", lastErrorMessage);
                lastRssUpdateResult = UpdateStatus::HTTP_ERROR;
                setNextFetchTime(millis() + 30000);
                return;
            }
            url = newUrl;
            redirectCount++;
        } else {
            snprintf(lastErrorMessage, sizeof(lastErrorMessage), "GET failed, error: %d %s", httpCode, http->errorToString(httpCode).c_str());
            LOG_WARN("DATA", lastErrorMessage);
            http->end();
            lastRssUpdateResult = UpdateStatus::HTTP_ERROR;
            setNextFetchTime(millis() + 30000);
            return;
        }
    }
    // never get here
    lastRssUpdateResult = UpdateStatus::HTTP_ERROR;
    setNextFetchTime(millis() + 30000);
    return;
}

/**
 * @brief Retrieves the last error message encountered during RSS fetch operations.
 * @return A string containing the error description.
 */
const char* rssClient::getLastError() const {
    return lastErrorMessage;
}

/**
 * @brief XML handler triggered for a start tag.
 * @param tag Name of the starting tag.
 */
void rssClient::startTag(const char *tag)
{
    tagLevel++;
    if (tagPath[0] != '\0') strlcat(tagPath, "/", sizeof(tagPath));
    int res = strlcat(tagPath, tag, sizeof(tagPath));
    if (res >= (int)sizeof(tagPath)) {
        LOG_ERRORf("DATA", "RSS Client: XML Path too long! (%s)", tagPath);
    }
}

/**
 * @brief XML handler triggered for an end tag.
 * @param tag Name of the ending tag.
 */
void rssClient::endTag(const char *tag)
{
    tagLevel--;
    char* lastSlash = strrchr(tagPath, '/');
    if (lastSlash) *lastSlash = '\0';
    else tagPath[0] = '\0';
}

/**
 * @brief XML handler triggered for a parameter.
 * @param param Parameter data.
 */
void rssClient::parameter(const char *param)
{
}

/**
 * @brief XML handler triggered for a text value.
 * @param value Scalar text value.
 */
void rssClient::value(const char *value)
{
    if (strstr(tagPath, "/item/title")) {
        strncpy(bgRssTitle[bgNumRssTitles],value,MAX_RSS_TITLE_SIZE-1);
        bgRssTitle[bgNumRssTitles][MAX_RSS_TITLE_SIZE-1] = '\0';
        trim(bgRssTitle[bgNumRssTitles]);
        LOG_DEBUGf("DATA", "RSS Item: %s", bgRssTitle[bgNumRssTitles]);
        bgNumRssTitles++;
    }
}

/**
 * @brief XML handler triggered for an attribute.
 * @param attr Attribute data.
 */
void rssClient::attribute(const char *attr)
{
}

/**
 * @brief Formats and appends current RSS headlines to the scrolling message pool.
 * @param pool Reference to the board-local MessagePool.
 * @param config Reference to global configuration for context-aware delimiters.
 */
void rssClient::addRssMessage(MessagePool& pool, const Config& config) {
    if (rssEnabled && pool.getCount() < 4 && numRssTitles > 0) {
        char combined[512];
        int len = snprintf(combined, sizeof(combined), "%s: %s", rssName, rssTitle[0]);
        
        for (int i = 1; i < numRssTitles; i++) {
            BoardTypes firstType = (config.boardCount > 0) ? config.boards[0].type : BoardTypes::MODE_RAIL;
            const char* delim = (firstType == BoardTypes::MODE_TUBE) ? "\x81" : "\x90";
            
            int nextNeeded = snprintf(NULL, 0, "%s%s", delim, rssTitle[i]);
            if (len + nextNeeded < (int)sizeof(combined)) {
                len += snprintf(combined + len, sizeof(combined) - len, "%s%s", delim, rssTitle[i]);
            } else {
                LOG_WARN("DATA", "RSS Client: Headlines truncated in message builder");
                break;
            }
        }
        pool.addMessage(combined);
        rssAddedtoMsgs = true;
    }
}

/**
 * @brief Clean up RSS messages from the board message pool.
 */
void rssClient::removeRssMessage(MessagePool& pool) {
    if (rssAddedtoMsgs) {
        pool.removeLastMessage();
        rssAddedtoMsgs = false;
    }
}
