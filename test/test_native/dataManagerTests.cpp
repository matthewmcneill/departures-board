#include <unity.h>
#include "dataManager.hpp"
#include "iDataSource.hpp"
#include <string>
#include <ArduinoFake.h>

using namespace fakeit;

class MockSource : public iDataSource {
public:
    MockSource(const char* name) : _name(name), fetchCount(0) {}
    
    UpdateStatus updateData() override { return UpdateStatus::SUCCESS; }
    void executeFetch() override { fetchCount++; }
    const char* getLastErrorMsg() const override { return ""; }
    uint32_t getNextFetchTime() override { return nextFetchTime; }
    void setNextFetchTime(uint32_t t) override { nextFetchTime = t; }
    PriorityTier getPriorityTier() override { return priorityTier; }
    UpdateStatus testConnection(const char* token = nullptr, const char* stationId = nullptr) override { return UpdateStatus::SUCCESS; }
    void serializeData(JsonObject& doc) override { doc["mock"] = true; }

    std::string _name;
    int fetchCount;
    uint32_t nextFetchTime = 0;
    PriorityTier priorityTier = PriorityTier::PRIO_MEDIUM;
};

void test_requestPriorityFetch() {
    dataManager dm;
    dm.init();
    MockSource source("SRC1");
    
    source.setNextFetchTime(100000); // 100s in the future
    
    // Mock millis() for this test
    When(Method(ArduinoFake(), millis)).AlwaysReturn(1000);
    
    // Request priority fetch
    uint32_t beforeMillis = millis();
    dm.requestPriorityFetch(&source);
    
    // Check if nextFetchTime was updated by dm
    TEST_ASSERT_TRUE(source.getNextFetchTime() <= beforeMillis + 500); 
}

void test_registration() {
    dataManager dm;
    MockSource s1("STATION_A");
    
    dm.registerSource(&s1);
    // Success if no crash
}

void runDataManagerTests() {
    RUN_TEST(test_requestPriorityFetch);
    RUN_TEST(test_registration);
}
