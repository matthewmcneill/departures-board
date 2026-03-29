#include <unity.h>
#include "dataManager.hpp"
#include "iDataSource.hpp"
#include <string>

class MockSource : public iDataSource {
public:
    MockSource(const char* name) : _name(name), fetchCount(0) {}
    
    const char* getSourceId() const override { return _name.c_str(); }
    void fetch() override { fetchCount++; }
    unsigned long getNextFetchTime() const override { return nextFetchTime; }
    void setNextFetchTime(unsigned long t) override { nextFetchTime = t; }
    int getPriorityTier() const override { return priorityTier; }

    std::string _name;
    int fetchCount;
    unsigned long nextFetchTime = 0;
    int priorityTier = 1;
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
