/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * TfL London Underground Client Library
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/TfLdataClient/TfLdataClient.h
 * Description: Exported functions and classes.
 *
 * Exported Functions/Classes:
 * - void: Void
 * - TfLdataClient: Class definition
 * - pruneFromPhrase: Prune from phrase
 * - replaceWord: Replace word
 * - compareTimes: Compare times
 * - updateArrivals: Update arrivals
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
typedef void (*tflClientCallback) ();

#define MAXLINESIZE 20
#define UGMAXREADSERVICES 20

class TfLdataClient: public JsonListener {

    private:

        struct ugService {
            char destinationName[MAXLOCATIONSIZE];
            char lineName[MAXLINESIZE];
            int timeToStation;
        };

        struct ugStation {
            int numServices;
            ugService service[UGMAXREADSERVICES];
        };

        const char* apiHost = "api.tfl.gov.uk";
        String currentKey = "";
        String currentObject = "";

        int id=0;
        bool maxServicesRead = false;
        ugStation xStation;
        stnMessages xMessages;

/**
 * @brief Prune from phrase
 * @param input
 * @param target
 * @return Return value
 */
        bool pruneFromPhrase(char* input, const char* target);
/**
 * @brief Replace word
 * @param input
 * @param target
 * @param replacement
 */
        void replaceWord(char* input, const char* target, const char* replacement);
/**
 * @brief Compare times
 * @param a
 * @param b
 * @return Return value
 */
        static bool compareTimes(const ugService& a, const ugService& b);

    public:
        String lastErrorMsg = "";

        TfLdataClient();
/**
 * @brief Update arrivals
 * @param station
 * @param messages
 * @param locationId
 * @param apiKey
 * @param Xcb
 * @return Return value
 */
        int updateArrivals(rdStation *station, stnMessages *messages, const char *locationId, String apiKey, tflClientCallback Xcb);

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