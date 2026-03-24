/*
 * Departures Board (c) 2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * Module: tools/layoutsim/src/weatherClient.hpp
 * Description: Mock weather client and status for the simulator.
 */

#ifndef WEATHER_CLIENT_MOCK_HPP
#define WEATHER_CLIENT_MOCK_HPP

#include "mockDataManager.hpp"

class WeatherStatus {
public:
    bool isValid() const { return true; }
    
    char getIconChar() const {
        int conditionId = MockDataManager::getInstance().getWeatherConditionId();
        bool isNight = MockDataManager::getInstance().getWeatherIsNight();

        if (conditionId == 800) return isNight ? 'B' : 'A';
        if (conditionId == 801 || conditionId == 802) return isNight ? 'D' : 'C';
        if (conditionId >= 803 && conditionId <= 804) return 'E';
        if (conditionId >= 200 && conditionId < 300) return 'G';
        if ((conditionId >= 300 && conditionId < 400) || (conditionId >= 500 && conditionId < 600)) return 'F';
        if (conditionId >= 600 && conditionId < 700) return 'H';
        if (conditionId >= 700 && conditionId < 800) return 'I';
        return '?';
    }
};

class WeatherClient {
private:
    WeatherStatus status;
public:
    WeatherStatus& getWeatherStatus() { return status; }
};

#endif
