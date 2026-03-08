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
 * Description: Client to fetch and parse SOAP XML requests from National Rail Darwin API.
 *
 * Exported Functions/Classes:
 * - class raildataXmlClient: Streaming XML parser and SOAP client for National Rail data.
 *   - raildataXmlClient(): Constructor.
 *   - init(): Fetches the SOAP host and API endpoint URL from WSDL definitions.
 *   - cleanFilter(): Normalizes a user-provided platform filter string.
 *   - updateDepartures(): Executes SOAP request, downloads and parses departure XML.
 *   - getLastError(): Retrieves the last error message.
 * - rdCallback: Type definition for progress callbacks.
 */

#pragma once
#include <xmlListener.h>
#include <xmlStreamingParser.h>
#include <stationData.h>

/**
 * @brief Callback function definition to report progress during long API initialization or updates.
 * @param state The current progress state.
 * @param id Optional identifier or count of items processed.
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

        char lastErrorMessage[128];
        bool firstDataLoad;
        bool endXml;

        char platformFilter[MAXPLATFORMFILTERSIZE];
        bool filterPlatforms = false;
        bool keepRoute = false;

        rdCallback Xcb;
/**
 * @brief Custom comparator to sort services chronologically.
 * @param a First service element.
 * @param b Second service element.
 * @return True if 'a' is chronologically earlier than 'b'.
 */
        static bool compareTimes(const rdiService& a, const rdiService& b);
/**
 * @brief Strips all HTML tags from a character array in-place.
 * @param input The character array.
 */
        void removeHtmlTags(char* input);
/**
 * @brief Replaces all occurrences of a target string with a replacement string in-place.
 * @param input The string buffer to modify.
 * @param target The exact word or phrase to look for.
 * @param replacement The string to inject.
 */
        void replaceWord(char* input, const char* target, const char* replacement);
/**
 * @brief Truncates a string from the first occurrence of a target phrase in-place.
 * @param input The character array.
 * @param target The string fragment to detect and prune from.
 */
        void pruneFromPhrase(char* input, const char* target);
/**
 * @brief Ensures a string ends with exactly one full stop in-place.
 * @param input The character array to normalize.
 */
        void fixFullStop(char* input);
/**
 * @brief Cleans HTML tags, ampersands, and unwanted text from scraped data payloads.
 */
        void sanitiseData();
/**
 * @brief Removes a service from the internal parsing array by its index and shifts subsequent elements.
 * @param x The array index to delete.
 */
        void deleteService(int x);
/**
 * @brief Trims leading and trailing whitespace from a character array in-place.
 * @param start Pointer to the start of the character array.
 * @param end Pointer to the end of the character array.
 */
        void trim(char* &start, char* &end);
/**
 * @brief Compares two character arrays case-insensitively.
 * @param a The first character array.
 * @param a_len The length of the first character array.
 * @param b The second character array (null-terminated).
 * @return True if the strings match regardless of case.
 */
        bool equalsIgnoreCase(const char* a, int a_len, const char* b);
/**
 * @brief Evaluates if a given service matches the configured platform filter.
 * @param filter The comma-separated string of desired platform numbers.
 * @param serviceId The currently parsed platform number.
 * @return True if the platform is found in the filter, or if the filter is empty.
 */
        bool serviceMatchesFilter(const char* filter, const char* serviceId);

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
        raildataXmlClient();
/**
 * @brief Fetches the SOAP host and API endpoint URL from the provided WSDL definitions.
 * @param wsdlHost The WSDL host domain.
 * @param wsdlAPI The WSDL API path.
 * @param RDcb Progress callback to run while loading data.
 * @return Connection status constant.
 */
        int init(const char *wsdlHost, const char *wsdlAPI, rdCallback RDcb);
/**
 * @brief Normalizes a user-provided filter string (e.g. platform numbers).
 * @param rawFilter The original filter input.
 * @param cleanedFilter The output buffer for the normalized filter.
 * @param maxLen The maximum length of the output buffer.
 */
        void cleanFilter(const char* rawFilter, char* cleanedFilter, size_t maxLen);
/**
 * @brief Main function to execute the SOAP request, download departure data, and parse the XML into the structured station format.
 * @param station Pointer to the station object to populate.
 * @param messages Pointer to the station messages object.
 * @param crsCode The station CRS code (e.g. EDB).
 * @param customToken The API auth token.
 * @param numRows Maximum number of services to fetch.
 * @param includeBusServices Whether to include bus replacement services.
 * @param callingCrsCode An optional filter CRS code for services traversing a specific destination.
 * @param platforms Comma-separated list of platform numbers to filter by.
 * @param timeOffset Time offset in minutes.
 * @return Connection status constant.
 */
        int updateDepartures(rdStation *station, stnMessages *messages, const char *crsCode, const char *customToken, int numRows, bool includeBusServices, const char *callingCrsCode, const char *platforms, int timeOffset);
/**
 * @brief Retrieves the last error message encountered during API operations.
 * @return A string containing the error.
 */
        const char* getLastError();
};