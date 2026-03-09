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
 * Description: Client to fetch and parse weather data from the OpenWeatherMap REST API.
 *
 * Provides:
 * - weatherClient: Streaming JSON parser and HTTP client to fetch weather data.
 * - currentWeather: Global instance pointer allocated onto the heap.
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
        
        char openWeatherMapApiKey[64] = "";
        char weatherMsg[46] = "";
        
        bool weatherEnabled = false;
        unsigned long nextWeatherUpdate = 0;

    public:
        String currentWeather = "";
        char lastErrorMsg[128];

        const char* getOpenWeatherMapApiKey() const { return openWeatherMapApiKey; }
        void setOpenWeatherMapApiKey(const char* newKey) { strncpy(openWeatherMapApiKey, newKey, sizeof(openWeatherMapApiKey)-1); }

        bool getWeatherEnabled() const { return weatherEnabled; }
        void setWeatherEnabled(bool val) { weatherEnabled = val; }

        unsigned long getNextWeatherUpdate() const { return nextWeatherUpdate; }
        void setNextWeatherUpdate(unsigned long val) { nextWeatherUpdate = val; }

        char* getWeatherMsg() { return weatherMsg; }
        void setWeatherMsg(const char* newMsg) { strncpy(weatherMsg, newMsg, sizeof(weatherMsg)-1); }

        weatherClient();

/**
 * @brief Connects to OpenWeatherMap API, retrieves the current weather for a location, and parses the JSON response.
 * @param apiKey The user's OpenWeatherMap API key.
 * @param lat The latitude of the location.
 * @param lon The longitude of the location.
 * @return True if the metadata was successfully fetched and parsed, otherwise false.
 */
        bool updateWeather(String apiKey, String lat, String lon);

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

extern weatherClient* currentWeather;