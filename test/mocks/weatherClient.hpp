#ifndef WEATHER_CLIENT_MOCK_HPP
#define WEATHER_CLIENT_MOCK_HPP

#include <Arduino.h>
#include "iConfigurable.hpp"
#include "iDataSource.hpp"
#include "weatherStatus.hpp"

class weatherClient : public iConfigurable, public iDataSource {
public:
    weatherClient() {}
    
    void reapplyConfig(const Config& config) override {}

    UpdateStatus updateData() override { return UpdateStatus::SUCCESS; }
    void executeFetch() override {}
    const char* getLastErrorMsg() const override { return ""; }
    uint32_t getNextFetchTime() override { return 0; }
    PriorityTier getPriorityTier() override { return PriorityTier::PRIO_LOW; }
    void setNextFetchTime(uint32_t forceTimeMillis) override {}
    UpdateStatus testConnection(const char* token = nullptr, const char* stationId = nullptr) override { return UpdateStatus::SUCCESS; }
    void serializeData(JsonObject& doc) override {}
    
    const WeatherStatus& getWeatherStatus() const { static WeatherStatus ws; return ws; }
};

#endif
