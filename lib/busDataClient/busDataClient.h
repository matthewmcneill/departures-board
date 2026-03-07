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
 * Description: Exported functions and classes.
 *
 * Exported Functions/Classes:
 * - void: Void
 * - busDataClient: Class definition
 * - stripTag: Strip tag
 * - replaceWord: Replace word
 * - trim: Trim
 * - equalsIgnoreCase: Equals ignore case
 * - serviceMatchesFilter: Service matches filter
 * - getStopLongName: Get stop long name
 * - cleanFilter: Clean filter
 * - updateDepartures: Update departures
 * - whitespace: Whitespace
 * - startDocument: Start document
 * - key: Key
 * - value: Value
 * - endArray: End array
 * - endObject: End object
 * - endDocument: End document
 * - startArray: Start array
 * - startObject: Start object
 */
#pragma once
#include <JsonListener.h>
#include <JsonStreamingParser.h>
#include <stationData.h>

/**
 * @brief Void
 * @param (
 * @return Return value
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
 * @brief Strip tag
 * @param html
 * @return Return value
 */
        String stripTag(String html);
/**
 * @brief Replace word
 * @param input
 * @param target
 * @param replacement
 */
        void replaceWord(char* input, const char* target, const char* replacement);
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

    public:
        String lastErrorMsg = "";

        busDataClient();
/**
 * @brief Get stop long name
 * @param locationId
 * @param locationName
 * @return Return value
 */
        int getStopLongName(const char *locationId, char *locationName);
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
 * @param locationId
 * @param filter
 * @param Xcb
 * @return Return value
 */
        int updateDepartures(rdStation *station, const char *locationId, const char *filter, busClientCallback Xcb);

/**
 * @brief Whitespace
 * @param c
 * @return Return value
 */
        virtual void whitespace(char c);
/**
 * @brief Start document
 * @return Return value
 */
        virtual void startDocument();
/**
 * @brief Key
 * @param key
 * @return Return value
 */
        virtual void key(String key);
/**
 * @brief Value
 * @param value
 * @return Return value
 */
        virtual void value(String value);
/**
 * @brief End array
 * @return Return value
 */
        virtual void endArray();
/**
 * @brief End object
 * @return Return value
 */
        virtual void endObject();
/**
 * @brief End document
 * @return Return value
 */
        virtual void endDocument();
/**
 * @brief Start array
 * @return Return value
 */
        virtual void startArray();
/**
 * @brief Start object
 * @return Return value
 */
        virtual void startObject();
};