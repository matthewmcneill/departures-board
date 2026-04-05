#include <unity.h>
#include "dataManager.hpp"
#include "iDataSource.hpp"
#include <string>

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

    std::string _name;
    int fetchCount;
    uint32_t nextFetchTime = 0;
    PriorityTier priorityTier = PriorityTier::PRIO_MEDIUM;
};

void test_requestPriorityFetch() {
    dataManager dm;
    MockSource source("SRC1");
    
    source.setNextFetchTime(100000); // 100s in the future
    
    // Request priority fetch
    dm.requestPriorityFetch(&source);
    
    // In our mock dm, it should set nextFetchTime to current or nearly current
    // Note: dm.init() might be needed if dm starts a task. 
    // In our FreeRTOS mock, it doesn't.
    
    // Check if nextFetchTime was updated by dm
    TEST_ASSERT_TRUE(source.getNextFetchTime() <= 5000); 
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
