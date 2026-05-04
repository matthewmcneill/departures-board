#ifndef RSS_CLIENT_MOCK_HPP
#define RSS_CLIENT_MOCK_HPP

#include <Arduino.h>

#include "iConfigurable.hpp"
#include "iDataSource.hpp"

class rssClient : public iConfigurable, public iDataSource {
public:
    rssClient() : rssEnabled(true), rssAddedtoMsgs(false), numRssTitles(0) {}
    bool getRssEnabled() const { return rssEnabled; }
    void setRssAddedtoMsgs(bool a) { rssAddedtoMsgs = a; }
    bool getRssAddedtoMsgs() const { return rssAddedtoMsgs; }
    const char* getRssName() const { return "MockRSS"; }
    
    void reapplyConfig(const Config& config) override {}

    UpdateStatus updateData() override { return UpdateStatus::SUCCESS; }
    void executeFetch() override {}
    const char* getLastErrorMsg() const override { return ""; }
    uint32_t getNextFetchTime() override { return 0; }
    PriorityTier getPriorityTier() override { return PriorityTier::PRIO_LOW; }
    void setNextFetchTime(uint32_t forceTimeMillis) override {}
    UpdateStatus testConnection(const char* token = nullptr, const char* stationId = nullptr) override { return UpdateStatus::SUCCESS; }
    void serializeData(JsonObject& doc) override {}

    bool rssEnabled;
    bool rssAddedtoMsgs;
    int numRssTitles;
    char rssTitle[5][100];
};

#endif
