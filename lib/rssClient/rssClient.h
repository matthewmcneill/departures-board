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

#pragma once
#include <xmlListener.h>
#include <xmlStreamingParser.h>

#define UPD_SUCCESS 0
#define UPD_INCOMPLETE 1
#define UPD_UNAUTHORISED 2
#define UPD_HTTP_ERROR 3
#define UPD_TIMEOUT 4
#define UPD_NO_RESPONSE 5
#define UPD_DATA_ERROR 6
#define UPD_NO_CHANGE 7

#define MAX_RSS_TITLES 5
#define MAX_RSS_TITLE_SIZE 140

class rssClient: public xmlListener {

    private:

        String grandParentTagName = "";
        String parentTagName = "";
        String tagName = "";
        String tagPath = "";
        int tagLevel = 0;
        String currentPath = "";
        char lastErrorMessage[128];
        
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

        rssClient();
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
};

extern rssClient* rss;