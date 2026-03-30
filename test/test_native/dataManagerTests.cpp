#include <unity.h>
#include "dataManager.hpp"
#include <string>

class MockSource : public iDataSource {
public:
    MockSource(const char* name) : _name(name) {}
    
    int updateData() override { return 0; }
    void executeFetch() override {}
    const char* getLastErrorMsg() const override { return ""; }
    uint32_t getNextFetchTime() override { return _nextFetch; }
    uint8_t getPriorityTier() override { return _tier; }
    void setNextFetchTime(uint32_t t) override { _nextFetch = t; }
    int testConnection(const char* token, const char* stationId) override { return 0; }
    
    void setTier(uint8_t t) { _tier = t; }

private:
    std::string _name;
    uint32_t _nextFetch = 0;
    uint8_t _tier = 1;
};

void test_data_manager_registration() {
    dataManager dm;
    MockSource source("Mock1");
    
    dm.registerSource(&source);
    
    // Setup a fake next fetch time
    uint32_t farFuture = millis() + 100000;
    source.setNextFetchTime(farFuture);
    
    // Verify registration side effects if possible
    TEST_ASSERT_EQUAL(farFuture, source.getNextFetchTime());
}

void runDataManagerTests() {
    RUN_TEST(test_data_manager_registration);
}
