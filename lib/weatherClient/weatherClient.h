/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * OpenWeatherMap Weather Client Library
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/weatherClient/weatherClient.h
 * Description: Exported functions and classes.
 *
 * Exported Functions/Classes:
 * - weatherClient: Class definition
 * - updateWeather: Update weather
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

class weatherClient: public JsonListener {

    private:
        const char* apiHost = "api.openweathermap.org";
        String currentKey = "";
        String currentObject = "";
        int weatherItem = 0;

        String description;
        float temperature;
        float windSpeed;

    public:
        String currentWeather = "";
        String lastErrorMsg = "";

        weatherClient();

/**
 * @brief Update weather
 * @param apiKey
 * @param lat
 * @param lon
 * @return Return value
 */
        bool updateWeather(String apiKey, String lat, String lon);

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