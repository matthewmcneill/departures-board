#ifndef WEATHER_CLIENT_MOCK_HPP
#define WEATHER_CLIENT_MOCK_HPP

#include <Arduino.h>

struct WeatherStatus {
    float lat;
    float lon;
    int temp;
    int conditionId;
    bool isNight;
};

class weatherClient {
public:
    weatherClient() : weatherEnabled(true) {}
    bool getWeatherEnabled() const { return weatherEnabled; }
    void updateWeather(WeatherStatus& ws, const char* key) {}
    
    bool weatherEnabled;
};

#endif
