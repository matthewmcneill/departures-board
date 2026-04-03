/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * OpenWeatherMap Weather Client Library
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/weatherClient/weatherClient.hpp
 * Description: Client to fetch and parse weather data from the OpenWeatherMap REST API.
 *              Stateless implementation that updates provided WeatherStatus objects.
 *
 * Exported Functions/Classes:
 * - weatherClient: [Class] Service for OpenWeatherMap integration.
 *   - updateWeather(): UI-driven manual sync.
 *   - executeFetch(): Task-worker entry for background fetching.
 *   - reapplyConfig(): Dynamic API key and preference sync.
 */
#pragma once
#include <Arduino.h>
#include <JsonStreamingParser.h>
#include "iConfigurable.hpp"
#include "weatherStatus.hpp"
#include "configManager.hpp"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include "../dataManager/iDataSource.hpp"

class weatherClient: public JsonListener, public iConfigurable, public iDataSource {

    private:
        const char* apiHost = "api.openweathermap.org";
        String currentKey = "";
        String currentObject = "";
        int weatherItem = 0;

        WeatherStatus bgStatus; // Background Double Buffer used during active HTTP parsing
        WeatherStatus* activeStatus = nullptr; // Pointer to the UI-facing active status structure
        String activeApiKey = ""; // Thread-local copy of the API key for background fetch
        
        volatile bool fetchPending = false; // Tracks if a fetch is queued or executing

        SemaphoreHandle_t weatherMutex; // Mutex protecting the memory copy from bgStatus to activeStatus
        
        char weatherMsg[46] = "";
        
        bool weatherEnabled = false;
        bool parsingComplete = false; // Indicates if the streaming parser has reached the end of the document
        unsigned long nextWeatherUpdate = 0;

    public:
        String currentWeather = "";
        char lastErrorMsg[128];

        // iDataSource boilerplate implementations (as updateWeather is used specifically)
        UpdateStatus updateData() override { return UpdateStatus::SUCCESS; }
        const char* getLastErrorMsg() const override { return lastErrorMsg; }
        UpdateStatus testConnection(const char* token = nullptr, const char* stationId = nullptr) override { return UpdateStatus::SUCCESS; }
        uint32_t getNextFetchTime() override { return nextWeatherUpdate; }
        PriorityTier getPriorityTier() override { return activeApiKey.length() == 0 ? static_cast<PriorityTier>(255) : PriorityTier::PRIO_LOW; } // Weather is low priority
        void setNextFetchTime(uint32_t forceTimeMillis) override { nextWeatherUpdate = forceTimeMillis; }

        bool getWeatherEnabled() const { return weatherEnabled; }
        void setWeatherEnabled(bool val) { weatherEnabled = val; }

        unsigned long getNextWeatherUpdate() const { return nextWeatherUpdate; }
        void setNextWeatherUpdate(unsigned long val) { nextWeatherUpdate = val; }

        char* getWeatherMsg() { return weatherMsg; }
        void setWeatherMsg(const char* newMsg) { strncpy(weatherMsg, newMsg, sizeof(weatherMsg)-1); }

        /**
         * @brief Check if a background fetch is currently queued or executing.
         */
        bool isFetchPending() const { return fetchPending; }

        /**
         * @brief Default constructor for the weather client.
         */
        weatherClient();

/**
 * @brief Connects to OpenWeatherMap API, retrieves the current weather for a location, and parses the JSON response.
 * @param status Reference to the WeatherStatus object to update.
 * @param apiKeyId The board's API key reference.
 * @param overrideToken Optional token for testing.
 * @return True if the metadata was successfully fetched and parsed, otherwise false.
 */
        bool updateWeather(WeatherStatus& status, const char* apiKeyId, const char* overrideToken = nullptr);

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
        virtual void startObject();
        virtual void reapplyConfig(const Config& config) override;

    public:
        void executeFetch() override;
};