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
 * - rssClient: XML/JSON news feed scroller with headline pool management.
 *   - addRssMessage(): Formats and pushes global headlines to the message pool.
 *   - removeRssMessage(): Clears active RSS content from the system.
 *   - loadFeed(): High-level parsing entry point.
 *   - setYieldCallback(): Registers a callback for non-blocking I/O.
 *   - reapplyConfig(): Updates RSS settings from central configuration.
 *   - executeFetch(): Internal synchronous HTTP pipeline.
 *   - fetchTask(): FreeRTOS static entry point for pinning network requests.
 */

#pragma once
#include <xmlListener.hpp>
#include <xmlStreamingParser.hpp>
#include "iConfigurable.hpp"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include <iDataSource.hpp>

#define MAX_RSS_TITLES 5
#define MAX_RSS_TITLE_SIZE 140

class MessagePool; // Forward declaration
struct Config;     // Forward declaration

class rssClient: public xmlListener, public iConfigurable, public iDataSource {

    private:

        String grandParentTagName = "";
        String parentTagName = "";
        String tagName = "";
        String tagPath = "";
        int tagLevel = 0;
        String currentPath = "";
        char lastErrorMessage[128];
        void (*yieldCallback)() = nullptr;
        
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
         * @brief Registers a callback function to be invoked during blocking I/O operations.
         * @param cb Pointer to the yield function.
         */
        void setYieldCallback(void (*cb)()) { yieldCallback = cb; }

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

    private:
};

extern rssClient* rss;