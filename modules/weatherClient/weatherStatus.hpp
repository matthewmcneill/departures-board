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

    /**
     * @brief Maps OpenWeatherMap condition IDs and day/night status to Font characters.
     *        Font: WeatherIcons16
     * @return char The character mapping to the icon.
     */
    char getIconChar() const {
        if (!isValid()) return '?';

        // OWM Groups:
        // 2xx: Thunderstorm
        // 3xx: Drizzle
        // 5xx: Rain
        // 6xx: Snow
        // 7xx: Atmosphere (Mist, Smoke, Haze, etc.)
        // 800: Clear
        // 80x: Clouds

        if (conditionId == 800) {
            return isNight ? 'B' : 'A'; // B: Clear Night, A: Clear Day
        }

        if (conditionId == 801 || conditionId == 802) {
            return isNight ? 'D' : 'C'; // D: Few Clouds Night, C: Few Clouds Day
        }

        if (conditionId >= 803 && conditionId <= 804) {
            return 'E'; // E: Clouds / Overcast
        }

        if (conditionId >= 200 && conditionId < 300) {
            return 'G'; // G: Thunderstorm
        }

        if ((conditionId >= 300 && conditionId < 400) || (conditionId >= 500 && conditionId < 600)) {
            return 'F'; // F: Rain / Shower
        }

        if (conditionId >= 600 && conditionId < 700) {
            return 'H'; // H: Snow
        }

        if (conditionId >= 700 && conditionId < 800) {
            return 'I'; // I: Atmosphere (Mist/Fog/Haze)
        }

        return '?';
    }
};
