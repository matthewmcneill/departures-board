#include <unity.h>
#include "configManager.hpp"
#include "FS.h"
#include "LittleFS.h"
#include <ArduinoJson.h>

void test_hasConfiguredBoards() {
    ConfigManager cm;
    
    // Initial state
    Config& c = cm.getConfig();
    c.boardCount = 0;
    TEST_ASSERT_FALSE(cm.hasConfiguredBoards());
    
    // Add an incomplete board
    c.boardCount = 1;
    c.boards[0].complete = false;
    TEST_ASSERT_FALSE(cm.hasConfiguredBoards());
    
    // Complete the board
    c.boards[0].complete = true;
    TEST_ASSERT_TRUE(cm.hasConfiguredBoards());
}

void test_v23_to_v24_migration() {
    ConfigManager cm;
    
    // Inject v2.3 legacy config into mock file system
    const char* v23_config = "{\"version\":2.3,\"turnOffOledInSleep\":true,\"boards\":[{\"type\":3,\"id\":\"CLK\",\"name\":\"Clock\"}]}";
    LittleFS._setFile("/config.json", v23_config);
    
    // Reset in-memory config to force reload
    cm.loadConfig();
    
    const Config& c = cm.getConfig();
    TEST_ASSERT_EQUAL_FLOAT(2.4f, c.configVersion);
    
    // Check migration of sleep flag (Board Type 3 is MODE_CLOCK)
    if (c.boardCount > 0 && c.boards[0].type == MODE_CLOCK) {
        TEST_ASSERT_TRUE(c.boards[0].oledOff);
    }
}

void test_save_load_roundtrip() {
    ConfigManager cm;
    
    // Setup state
    Config& c = cm.getConfig();
    strcpy(c.hostname, "TestHost");
    c.brightness = 150;
    
    // Save to mock FS
    TEST_ASSERT_TRUE(cm.save());
    
    // Reload
    cm.loadConfig();
    
    // Verify
    TEST_ASSERT_EQUAL_STRING("TestHost", cm.getConfig().hostname);
    TEST_ASSERT_EQUAL(150, cm.getConfig().brightness);
}

void runConfigManagerTests() {
    RUN_TEST(test_hasConfiguredBoards);
    RUN_TEST(test_v23_to_v24_migration);
    RUN_TEST(test_save_load_roundtrip);
}
