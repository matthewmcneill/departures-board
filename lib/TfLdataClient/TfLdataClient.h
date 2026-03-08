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
 * Description: Client to interact with the Transport for London (TfL) Unified API.
 *
 * Exported Functions/Classes:
 * - class TfLdataClient: Main JSON parsing and request engine for TfL data.
 *   - TfLdataClient(): Constructor.
 *   - updateArrivals(): Connects to TfL API, requests arrival times and disruptions, and parses JSON.
 *   - lastErrorMsg: Attribute containing the last error message from API operations.
 * - tflClientCallback: Type definition for progress callbacks.
 */
#pragma once
#include <JsonListener.h>
#include <JsonStreamingParser.h>
#include <stationData.h>

/**
 * @brief Callback function definition to execute periodic updates (e.g. flashing UI cursors) during long API waits.
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
 * @brief Trims a character array by terminating it at the first occurrence of a target phrase.
 * @param input The character array to prune.
 * @param target The substring indicating where pruning should begin.
 * @return True if the target was found and pruned, otherwise false.
 */
        bool pruneFromPhrase(char* input, const char* target);
/**
 * @brief Replaces all occurrences of a target string with a replacement string in-place.
 * @param input The string buffer to modify.
 * @param target The exact word or phrase to look for.
 * @param replacement The string to inject in place of the target.
 */
        void replaceWord(char* input, const char* target, const char* replacement);
/**
 * @brief Comparator function to sort arrival times in ascending chronological order.
 * @param a The first service entry.
 * @param b The second service entry.
 * @return True if a arrives sooner than b.
 */
        static bool compareTimes(const ugService& a, const ugService& b);

    public:
        char lastErrorMsg[128];

        TfLdataClient();
/**
 * @brief Connects to the TfL API, requests arrival times and disruptions for a StopPoint, and parses the JSON response.
 * @param station Pointer to the global rdStation structure to populate with arrival times.
 * @param messages Pointer to the global stnMessages structure to populate with service status disruptions.
 * @param locationId The TfL StopPoint Naptan ID.
 * @param apiKey The TfL Unified API token.
 * @param Xcb Function callback to execute UI ticks while blocking and waiting for the API.
 * @return A connection status constant (e.g. UPD_SUCCESS, UPD_TIMEOUT, etc.).
 */
        int updateArrivals(rdStation *station, stnMessages *messages, const char *locationId, String apiKey, tflClientCallback Xcb);

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