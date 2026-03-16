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
 * Description: Implementation of the RSS XML client.
 *
 * Exported Functions/Classes:
 * - rssClient::rssClient: Constructor.
 * - rssClient::loadFeed: Fetches RSS XML and extracts titles.
 * - rssClient::reapplyConfig: Provisions RSS settings from global configuration.
 */

#include <rssClient.hpp>
#include <xmlListener.hpp>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <logger.hpp>
#include <memory>

/**
 * @brief Implements the iConfigurable interface.
 */
void rssClient::reapplyConfig(const Config& config) {
    setRssURL(config.rssUrl);
    setRssName(config.rssName);
    setRssEnabled(config.rssEnabled);
    LOG_INFO("CONFIG", "RSS Config: Enabled=" + String(config.rssEnabled ? "true" : "false") + ", URL=" + String(config.rssUrl));
}

rssClient::rssClient() {}

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
 * @brief Connects to the provided URL, fetches the RSS feed, and streams the XML to extract item titles.
 * @param url The URL of the RSS feed to fetch (supports HTTP and HTTPS).
 * @return Connection status constant.
 */
int rssClient::loadFeed(String url) {
    std::unique_ptr<HTTPClient> http(new (std::nothrow) HTTPClient());
    std::unique_ptr<WiFiClient> client(new (std::nothrow) WiFiClient());
    std::unique_ptr<WiFiClientSecure> clientSecure(new (std::nothrow) WiFiClientSecure());

    if (!http || !client || !clientSecure) {
        LOG_ERROR("DATA", "RSS Client: Memory allocation failed for clients!");
        return UPD_DATA_ERROR;
    }

    unsigned long perfTimer = millis();
    int redirectCount = 0;
    const int maxRedirects = 5;

    // --- Step 1: Initialization ---
    strcpy(lastErrorMessage, "Success");
    clientSecure->setInsecure();
    http->setReuse(false);
    numRssTitles = 0;
    LOG_INFO("DATA", "RSS Client: Loading feed from " + url);

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
                return UPD_DATA_ERROR;
            }
            parser->setListener(this);
            parser->reset();
            grandParentTagName = "";
            parentTagName = "";
            tagName = "";
            tagLevel = 0;
            long dataReceived = 0;
            char c;
            unsigned long dataSendTimeout = millis() + 3000UL;

            // --- Step 3: Streaming Parse ---
            while((stream->available() || http->connected()) && millis() < dataSendTimeout && numRssTitles < MAX_RSS_TITLES) {
                while (stream->available() && numRssTitles < MAX_RSS_TITLES) {
                    c = stream->read();
                    parser->parse(c);
                    dataReceived++;
                }
                if (yieldCallback) yieldCallback();
                delay(1);
            }

            http->end();
            if (millis() >= dataSendTimeout) {
                snprintf(lastErrorMessage, sizeof(lastErrorMessage), "Timed out during data receive operation - %ld bytes received", dataReceived);
                LOG_WARN("DATA", lastErrorMessage);
                return UPD_TIMEOUT;
            }
            snprintf(lastErrorMessage, sizeof(lastErrorMessage), "Success: %ld bytes took %lums with %d redirects", dataReceived, static_cast<unsigned long>(millis()-perfTimer), redirectCount);
            LOG_INFO("DATA", "RSS Feed: Successfully fetched " + String(dataReceived) + " bytes. Found " + String(numRssTitles) + " titles.");
            return UPD_SUCCESS;
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
                return UPD_HTTP_ERROR;
                break;
            }
            url = newUrl;
            redirectCount++;
        } else {
            snprintf(lastErrorMessage, sizeof(lastErrorMessage), "GET failed, error: %d %s", httpCode, http->errorToString(httpCode).c_str());
            LOG_WARN("DATA", lastErrorMessage);
            http->end();
            return UPD_HTTP_ERROR;
            break;
        }
    }
    // never get here
    return UPD_SUCCESS;
}

/**
 * @brief Retrieves the last error message encountered during RSS fetch operations.
 * @return A string containing the error description.
 */
const char* rssClient::getLastError() {
    return lastErrorMessage;
}

/**
 * @brief XML handler triggered for a start tag.
 * @param tag Name of the starting tag.
 */
void rssClient::startTag(const char *tag)
{
    tagLevel++;
    grandParentTagName = parentTagName;
    parentTagName = tagName;
    tagName = String(tag);
    tagPath = grandParentTagName + "/" + parentTagName + "/" + tagName;
}

/**
 * @brief XML handler triggered for an end tag.
 * @param tag Name of the ending tag.
 */
void rssClient::endTag(const char *tag)
{
    tagLevel--;
    tagName = parentTagName;
    parentTagName=grandParentTagName;
    grandParentTagName="??";
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
    if (tagPath.endsWith("/item/title")) {
        strncpy(rssTitle[numRssTitles],value,MAX_RSS_TITLE_SIZE-1);
        rssTitle[numRssTitles][MAX_RSS_TITLE_SIZE-1] = '\0';
        trim(rssTitle[numRssTitles]);
        LOG_DEBUG("DATA", "RSS Item: " + String(rssTitle[numRssTitles]));
        numRssTitles++;
    }
}

/**
 * @brief XML handler triggered for an attribute.
 * @param attr Attribute data.
 */
void rssClient::attribute(const char *attr)
{
}
