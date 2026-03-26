/*
 * Departures Board (c) 2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * Module: tools/layoutsim/src/weatherClient.hpp
 * Description: Mock weather client and status for the simulator.
 */

#ifndef WEATHER_CLIENT_MOCK_HPP
#define WEATHER_CLIENT_MOCK_HPP

#include <weatherStatus.hpp>

class WeatherClient {
private:
    WeatherStatus status;
public:
    WeatherStatus& getWeatherStatus() { return status; }
};

#endif
