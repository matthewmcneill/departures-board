/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/weatherClient/weatherStatus.hpp
 * Description: Container for weather data associated with a specific location/board.
 *              Uses integer condition IDs aligned with OpenWeatherMap.
 *
 * Exported Functions/Classes:
 * - WeatherStatus: Data container for weather condition, temp, wind, and identity.
 * - WeatherUpdateStatus: Enum for data validity tracking.
 */

#pragma once

#include <Arduino.h>

/**
 * @brief Represents the data retrieval status for weather.
 */
enum class WeatherUpdateStatus {
    NO_DATA,         // Never updated
    READY,           // Valid data present
    NOT_CONFIGURED,  // Missing API key or coordinates
    DATA_ERROR       // Retrieval failed
};

/**
 * @brief Container for weather information.
 *        Owned by iDisplayBoard instances to support multi-location weather.
 */
class WeatherStatus {
public:
    WeatherUpdateStatus status = WeatherUpdateStatus::NO_DATA;
    
    // --- Identity ---
    float lat = 0.0f;
    float lon = 0.0f;

    // --- Core Data ---
    int conditionId = 0;       // OWM condition ID (e.g. 800 for Clear)
    bool isNight = false;      // True if the 'n' suffix was in the OWM icon code
    float temp = 0.0f;         // Celsius
    float windSpeed = 0.0f;    // knots or m/s (default knt)
    char description[46] = ""; // Textual description (e.g. "Clear Sky")

    /**
     * @brief Checks if the object contains valid, displayable weather data.
     */
    bool isValid() const {
        return status == WeatherUpdateStatus::READY;
    }

    /**
     * @brief Formats current temp and wind for display.
     */
    String toString() const {
        if (!isValid()) return "Weather: N/A";
        return String(description) + " " + String((int)temp) + "C, " + String((int)windSpeed) + "knt";
    }
};
