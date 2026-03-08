/*
 * Departures Board (c) 2025-2026 Gadec Software
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
 * - rssClient::trim: Trims leading and trailing whitespace from a character array.
 * - rssClient::loadFeed: Connects to the URL and extracts RSS item titles.
 * - rssClient::getLastError: Retrieves the last error message.
 * - rssClient::startTag: XML handler triggered for a start tag.
 * - rssClient::endTag: XML handler triggered for an end tag.
 * - rssClient::parameter: XML handler triggered for a parameter.
 * - rssClient::value: XML handler triggered for a text value.
 * - rssClient::attribute: XML handler triggered for an attribute.
 */

#include <rssClient.h>
#include <xmlListener.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>

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
    HTTPClient http;
    WiFiClient client;
    WiFiClientSecure clientSecure;

    unsigned long perfTimer = millis();
    int redirectCount = 0;
    const int maxRedirects = 5;

    lastErrorMessage = F("Success");
    clientSecure.setInsecure();
    http.setReuse(false);
    numRssTitles = 0;

    while (redirectCount < maxRedirects) {
        if (url.startsWith(F("https"))) http.begin(clientSecure,url);
        else http.begin(client, url);
        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK) {
            WiFiClient *stream = http.getStreamPtr();
            xmlStreamingParser parser;
            parser.setListener(this);
            parser.reset();
            grandParentTagName = "";
            parentTagName = "";
            tagName = "";
            tagLevel = 0;
            long dataReceived = 0;
            char c;
            unsigned long dataSendTimeout = millis() + 3000UL;

            while((stream->available() || http.connected()) && millis() < dataSendTimeout && numRssTitles < MAX_RSS_TITLES) {
                while (stream->available() && numRssTitles < MAX_RSS_TITLES) {
                    c = stream->read();
                    parser.parse(c);
                    dataReceived++;
                }
                delay(1);
            }

            http.end();
            if (millis() >= dataSendTimeout) {
                lastErrorMessage += F("Timed out during data receive operation - ");
                lastErrorMessage += String(dataReceived) + F(" bytes received");
                return UPD_TIMEOUT;
            }
            lastErrorMessage = "Success: " + String(dataReceived) + F(" bytes took ") + String(millis()-perfTimer) + F("ms with ") + String(redirectCount) + F(" redirects");
            return UPD_SUCCESS;
            break;
        } else if (httpCode == HTTP_CODE_MOVED_PERMANENTLY ||
                   httpCode == HTTP_CODE_FOUND ||
                   httpCode == HTTP_CODE_TEMPORARY_REDIRECT ||
                   httpCode == HTTP_CODE_PERMANENT_REDIRECT) {
            // Handle redirect
            String newUrl = http.getLocation();
            http.end();  // End current request before retrying
            if (newUrl.length() == 0) {
                lastErrorMessage = F("HTTP Redirect without Location header!");
                return UPD_HTTP_ERROR;
                break;
            }
            url = newUrl;
            redirectCount++;
        } else {
            lastErrorMessage = "GET failed, error: " + String(httpCode) + " " + http.errorToString(httpCode);
            http.end();
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
String rssClient::getLastError() {
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
