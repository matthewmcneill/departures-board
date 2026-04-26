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
 * Module: lib/rssClient/rssClient.hpp
 * Description: Client to fetch and parse RSS feeds via HTTP/HTTPS.
 *
 * Exported Functions/Classes:
 * - rssClient: [Class] XML news feed manager.
 *   - loadFeed(): Strategic entry point for manual headline sync.
 *   - updateData(): Background-driven schedule update.
 *   - executeFetch(): Worker-thread background fetch implementation with snprintf.
 *   - addRssMessage(): Syncs headlines to the global MessagePool using char buffers.
 *   - startTag() / endTag(): XML path tracking using fixed-size char buffers.
 */

#pragma once
#include <xmlListener.hpp>
#include "iConfigurable.hpp"
#include <freertos/FreeRTOS.h>
#include <Arduino.h>
#include <HTTPClient.h>
#include <xmlStreamingParser.hpp>
#include "xmlListener.hpp"
#include <freertos/task.h>
#include <freertos/semphr.h>

#include <iDataSource.hpp>

#define MAX_RSS_TITLES 5
#define MAX_RSS_TITLE_SIZE 140

class MessagePool; // Forward declaration
struct Config;     // Forward declaration

class rssClient: public xmlListener, public iConfigurable, public iDataSource {

    private:

        char tagPath[64] = "";
        int tagLevel = 0;
        char lastErrorMessage[128];
        
        bool rssEnabled = false;
        bool rssAddedtoMsgs = false;
        uint32_t nextFetchTimeMillis = 0;
        volatile UpdateStatus lastRssUpdateResult = UpdateStatus::SUCCESS;
        char rssURL[128] = "";
        char rssName[48] = "";

        char bgRssTitle[MAX_RSS_TITLES][MAX_RSS_TITLE_SIZE]; // Background storage for active XML extraction
        int bgNumRssTitles = 0; // Length counter for background array

        SemaphoreHandle_t rssMutex; // Thread-safe memory copy protection lock
        
        String activeUrl = ""; // Thread-local scoped copy of target URL

/**
 * @brief Trims leading and trailing whitespace from a character array in-place.
 * @param str The character array to trim.
 */
        void trim(char* str);

/**
 * @brief XML handler triggered for a start tag.
 * @param tagName Name of the starting tag.
 */
        virtual void startTag(const char *tagName);
/**
 * @brief XML handler triggered for an end tag.
 * @param tagName Name of the ending tag.
 */
        virtual void endTag(const char *tagName);
/**
 * @brief XML handler triggered for a parameter.
 * @param param Parameter data.
 */
        virtual void parameter(const char *param);
/**
 * @brief XML handler triggered for a text value.
 * @param value Scalar text value.
 */
        virtual void value(const char *value);
/**
 * @brief XML handler triggered for an attribute.
 * @param attribute Attribute data.
 */
        virtual void attribute(const char *attribute);

    public:

        bool getRssEnabled() const { return rssEnabled; }
        void setRssEnabled(bool val) { rssEnabled = val; }

        bool getRssAddedtoMsgs() const { return rssAddedtoMsgs; }
        void setRssAddedtoMsgs(bool val) { rssAddedtoMsgs = val; }

        /**
         * @brief Retrieve the required display attribution for the active feed.
         * @return Pointer to the RSS Name string.
         */
        const char* getAttributionString() const override { return rssName; }

        unsigned long getNextRssUpdate() const { return nextFetchTimeMillis; }
        void setNextRssUpdate(unsigned long val) { nextFetchTimeMillis = val; }

        UpdateStatus getLastRssUpdateResult() const { return lastRssUpdateResult; }
        void setLastRssUpdateResult(UpdateStatus val) { lastRssUpdateResult = val; }

        const char* getRssURL() const { return rssURL; }
        void setRssURL(const char* url) { strncpy(rssURL, url, sizeof(rssURL)-1); }

        const char* getRssName() const { return rssName; }
        void setRssName(const char* name) { strncpy(rssName, name, sizeof(rssName)-1); }

        /**
         * @brief Default constructor for the RSS client.
         */
        rssClient();

/**
 * @brief Connects to the provided URL, fetches the RSS feed, and streams the XML to extract item titles.
 * @param url The URL of the RSS feed to fetch (supports HTTP and HTTPS).
 * @return Connection status constant.
 */
        UpdateStatus loadFeed(String url);
/**
 * @brief Retrieves the last error message encountered during RSS fetch operations.
 * @return A string containing the error description.
 */
        const char* getLastError() const;
        char rssTitle[MAX_RSS_TITLES][MAX_RSS_TITLE_SIZE];
        int numRssTitles = 0;

        /**
         * @brief Implements the iConfigurable interface.
         */
        virtual void reapplyConfig(const Config& config) override;

        /**
         * @brief Append RSS headlines to the scrolling message pool.
         */
        void addRssMessage(MessagePool& pool, const Config& config);

        /**
         * @brief Clean up RSS messages from the board message pool.
         */
        void removeRssMessage(MessagePool& pool);

        // --- iDataSource Interface Methods ---
        UpdateStatus updateData() override;
        UpdateStatus testConnection(const char* token = nullptr, const char* stationId = nullptr) override;
        uint32_t getNextFetchTime() override { return nextFetchTimeMillis; }
        PriorityTier getPriorityTier() override { return PriorityTier::PRIO_LOW; } // RSS is low priority background data
        void setNextFetchTime(uint32_t forceTimeMillis) override { nextFetchTimeMillis = forceTimeMillis; }
        const char* getLastErrorMsg() const override { return getLastError(); }
        void executeFetch() override;
        void serializeData(JsonObject& doc) override;

    private:
};

extern rssClient* rss;