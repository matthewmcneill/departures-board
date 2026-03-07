/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * raildataXmlClient Library
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/raildataXmlClient/raildataXmlClient.h
 * Description: Exported functions and classes.
 *
 * Exported Functions/Classes:
 * - void: Void
 * - raildataXmlClient: Class definition
 * - compareTimes: Compare times
 * - removeHtmlTags: Remove html tags
 * - replaceWord: Replace word
 * - pruneFromPhrase: Prune from phrase
 * - fixFullStop: Fix full stop
 * - sanitiseData: Sanitise data
 * - deleteService: Delete service
 * - trim: Trim
 * - equalsIgnoreCase: Equals ignore case
 * - serviceMatchesFilter: Service matches filter
 * - startTag: Start tag
 * - endTag: End tag
 * - parameter: Parameter
 * - value: Value
 * - attribute: Attribute
 * - init: Init
 * - cleanFilter: Clean filter
 * - updateDepartures: Update departures
 * - getLastError: Get last error
 */

#pragma once
#include <xmlListener.h>
#include <xmlStreamingParser.h>
#include <stationData.h>

/**
 * @brief Void
 * @param state
 * @param id
 * @return Return value
 */
typedef void (*rdCallback) (int state, int id);

#define MAXHOSTSIZE 48
#define MAXAPIURLSIZE 48
#define MAXPLATFORMFILTERSIZE 25


class raildataXmlClient: public xmlListener {

    private:

        struct rdiService {
          char sTime[6];
          char destination[MAXLOCATIONSIZE];
          char via[MAXLOCATIONSIZE];
          char origin[MAXLOCATIONSIZE];
          char etd[11];
          char platform[4];
          bool isCancelled;
          bool isDelayed;
          int trainLength;
          byte classesAvailable;
          char opco[50];
          char calling[MAXCALLINGSIZE];
          char serviceMessage[MAXMESSAGESIZE];
          int serviceType;
        };

        struct rdiStation {
          char location[MAXLOCATIONSIZE];
          bool platformAvailable;
          int numServices;
          rdiService service[MAXBOARDSERVICES];
        };

        String grandParentTagName = "";
        String parentTagName = "";
        String tagName = "";
        String tagPath = "";
        int tagLevel = 0;
        bool loadingWDSL=false;
        String soapURL = "";
        char soapHost[MAXHOSTSIZE];
        char soapAPI[MAXAPIURLSIZE];

        String currentPath = "";
        rdiStation xStation;
        stnMessages xMessages;

        bool addedStopLocation = false;
        int id=0;
        int coaches=0;

        String lastErrorMessage = "";
        bool firstDataLoad;
        bool endXml;

        char platformFilter[MAXPLATFORMFILTERSIZE];
        bool filterPlatforms = false;
        bool keepRoute = false;

        rdCallback Xcb;
/**
 * @brief Compare times
 * @param a
 * @param b
 * @return Return value
 */
        static bool compareTimes(const rdiService& a, const rdiService& b);
/**
 * @brief Remove html tags
 * @param input
 */
        void removeHtmlTags(char* input);
/**
 * @brief Replace word
 * @param input
 * @param target
 * @param replacement
 */
        void replaceWord(char* input, const char* target, const char* replacement);
/**
 * @brief Prune from phrase
 * @param input
 * @param target
 */
        void pruneFromPhrase(char* input, const char* target);
/**
 * @brief Fix full stop
 * @param input
 */
        void fixFullStop(char* input);
/**
 * @brief Sanitise data
 */
        void sanitiseData();
/**
 * @brief Delete service
 * @param x
 */
        void deleteService(int x);
/**
 * @brief Trim
 * @param start
 * @param end
 */
        void trim(char* &start, char* &end);
/**
 * @brief Equals ignore case
 * @param a
 * @param a_len
 * @param b
 * @return Return value
 */
        bool equalsIgnoreCase(const char* a, int a_len, const char* b);
/**
 * @brief Service matches filter
 * @param filter
 * @param serviceId
 * @return Return value
 */
        bool serviceMatchesFilter(const char* filter, const char* serviceId);

/**
 * @brief Start tag
 * @param tagName
 * @return Return value
 */
        virtual void startTag(const char *tagName);
/**
 * @brief End tag
 * @param tagName
 * @return Return value
 */
        virtual void endTag(const char *tagName);
/**
 * @brief Parameter
 * @param param
 * @return Return value
 */
        virtual void parameter(const char *param);
/**
 * @brief Value
 * @param value
 * @return Return value
 */
        virtual void value(const char *value);
/**
 * @brief Attribute
 * @param attribute
 * @return Return value
 */
        virtual void attribute(const char *attribute);

    public:
        raildataXmlClient();
/**
 * @brief Init
 * @param wsdlHost
 * @param wsdlAPI
 * @param RDcb
 * @return Return value
 */
        int init(const char *wsdlHost, const char *wsdlAPI, rdCallback RDcb);
/**
 * @brief Clean filter
 * @param rawFilter
 * @param cleanedFilter
 * @param maxLen
 */
        void cleanFilter(const char* rawFilter, char* cleanedFilter, size_t maxLen);
/**
 * @brief Update departures
 * @param station
 * @param messages
 * @param crsCode
 * @param customToken
 * @param numRows
 * @param includeBusServices
 * @param callingCrsCode
 * @param platforms
 * @param timeOffset
 * @return Return value
 */
        int updateDepartures(rdStation *station, stnMessages *messages, const char *crsCode, const char *customToken, int numRows, bool includeBusServices, const char *callingCrsCode, const char *platforms, int timeOffset);
/**
 * @brief Get last error
 * @return Return value
 */
        String getLastError();
};