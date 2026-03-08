/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * bustimes.org Client Library
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/busDataClient/busDataClient.h
 * Description: Client to scrape and parse bus departure data from bustimes.org.
 *
 * Exported Functions/Classes:
 * - class busDataClient: Parsing and HTML scraping engine for bus data.
 *   - busDataClient(): Constructor.
 *   - getStopLongName(): Retrieves the official long name of a bus stop.
 *   - cleanFilter(): Normalizes a user-provided filter string.
 *   - updateDepartures(): Scrapes and parses the HTML departures board.
 *   - lastErrorMsg: Attribute containing the last error message from API operations.
 * - busClientCallback: Type definition for progress callbacks.
 */
#pragma once
#include <JsonListener.h>
#include <JsonStreamingParser.h>
#include <stationData.h>

/**
 * @brief Callback function definition to execute periodic updates (e.g. flashing UI cursors) during long API waits.
 */
typedef void (*busClientCallback) ();

#define MAXBUSLINESIZE 9
#define BUSMAXREADSERVICES 20
#define MAXBUSFILTERSIZE 25

#define PBT_START 0
#define PBT_HEADER 1
#define PBT_SERVICE 2
#define PBT_DESTINATION 3
#define PBT_SCHEDULED 4
#define PBT_EXPECTED 5

class busDataClient: public JsonListener {

    private:

        struct busService {
            char destinationName[MAXLOCATIONSIZE];
            char lineName[MAXBUSLINESIZE];
            char scheduled[6];
            char expected[6];
        };

        struct busStop {
            int numServices;
            busService service[BUSMAXREADSERVICES];
        };

        const char* apiHost = "bustimes.org";
        String currentKey = "";
        String currentObject = "";

        int id=0;
        String longName;
        bool maxServicesRead = false;
        busStop xBusStop;

/**
 * @brief Extracts the inner text from an HTML tag string.
 * @param html The full HTML tag string to parse.
 * @return The inner text stripped of its surrounding tags.
 */
        String stripTag(String html);
/**
 * @brief Replaces all occurrences of a target string with a replacement string in-place.
 * @param input The string buffer to modify.
 * @param target The exact word or phrase to look for.
 * @param replacement The string to inject in place of the target.
 */
        void replaceWord(char* input, const char* target, const char* replacement);
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
 * @brief Checks if a specific bus service ID matches a provided comma-separated filter list.
 * @param filter The comma-separated string of desired bus route numbers.
 * @param serviceId The currently parsed bus route number.
 * @return True if the route is found in the filter, or if the filter is empty.
 */
        bool serviceMatchesFilter(const char* filter, const char* serviceId);

    public:
        char lastErrorMsg[128];

        busDataClient();
/**
 * @brief Connects to bustimes.org API to retrieve the official long name of a bus stop.
 * @param locationId The stop ID.
 * @param locationName Output buffer to store the retrieved name.
 * @return A connection status constant (e.g. UPD_SUCCESS, UPD_TIMEOUT).
 */
        int getStopLongName(const char *locationId, char *locationName);
/**
 * @brief Normalizes a user-provided filter string by making it lowercase and removing spaces.
 * @param rawFilter The original filter input.
 * @param cleanedFilter The output buffer for the normalized filter.
 * @param maxLen The maximum length of the output buffer.
 */
        void cleanFilter(const char* rawFilter, char* cleanedFilter, size_t maxLen);
/**
 * @brief Connects to bustimes.org, scrapes the HTML departures board, and parses the vehicle times.
 * @param station Pointer to the global rdStation structure to populate with arrival times.
 * @param locationId The bustimes.org Stop ID.
 * @param filter A comma-separated list of bus routes to filter by (or empty for all).
 * @param Xcb Function callback to execute UI ticks while blocking and waiting for the API.
 * @return A connection status constant.
 */
        int updateDepartures(rdStation *station, const char *locationId, const char *filter, busClientCallback Xcb);

/**
 * @brief JSON whitespace handler.
 * @param c The whitespace char.
 */
        virtual void whitespace(char c);

/**
 * @brief JSON handler triggered at start of document.
 */
        virtual void startDocument();

/**
 * @brief JSON handler triggered for each object key.
 * @param key The string name of the parsed key.
 */
        virtual void key(String key);

/**
 * @brief JSON handler triggered for each key value.
 * @param value The scalar string value.
 */
        virtual void value(String value);

/**
 * @brief JSON handler triggered when exiting an array.
 */
        virtual void endArray();

/**
 * @brief JSON handler triggered when exiting an object.
 */
        virtual void endObject();

/**
 * @brief JSON handler triggered at end of document.
 */
        virtual void endDocument();

/**
 * @brief JSON handler triggered when entering an array.
 */
        virtual void startArray();

/**
 * @brief JSON handler triggered when entering an object.
 */
        virtual void startObject();
};