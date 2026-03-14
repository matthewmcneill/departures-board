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
 * Module: lib/rssClient/rssClient.h
 * Description: Client to fetch and parse RSS feeds via HTTP/HTTPS.
 *
 * Exported Functions/Classes:
 * - rssClient: Main service class for RSS feed integration.
 * - rssClient::loadFeed: Fetches and parses an RSS feed from a URL.
 * - rssClient::setYieldCallback: Registers a callback for non-blocking I/O.
 * - rssClient::reapplyConfig: Updates RSS settings from central configuration.
 */

#pragma once
#include <xmlListener.h>
#include <xmlStreamingParser.h>
#include "iConfigurable.hpp"

#include <boards/interfaces/iDataSource.hpp>

#define MAX_RSS_TITLES 5
#define MAX_RSS_TITLE_SIZE 140

class rssClient: public xmlListener, public iConfigurable {

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
        unsigned long nextRssUpdate = 0;
        int lastRssUpdateResult = 0;
        char rssURL[128] = "";
        char rssName[48] = "";

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

        unsigned long getNextRssUpdate() const { return nextRssUpdate; }
        void setNextRssUpdate(unsigned long val) { nextRssUpdate = val; }

        int getLastRssUpdateResult() const { return lastRssUpdateResult; }
        void setLastRssUpdateResult(int val) { lastRssUpdateResult = val; }

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
        int loadFeed(String url);
/**
 * @brief Retrieves the last error message encountered during RSS fetch operations.
 * @return A string containing the error description.
 */
        const char* getLastError();
        char rssTitle[MAX_RSS_TITLES][MAX_RSS_TITLE_SIZE];
        int numRssTitles = 0;

        /**
         * @brief Implements the iConfigurable interface.
         */
        virtual void reapplyConfig(const Config& config) override;
};

extern rssClient* rss;