#include <unity.h>
#include <configManager.hpp>
#include <appContext.hpp>
#include <FS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

void test_hasConfiguredBoards() {
    appContext ctx;
    ConfigManager& cm = ctx.getConfigManager();
    
    // Clear state
    cm.loadConfig(); // Will load nothing, set defaults
    
    // Initial state: boardCount = 0
    TEST_ASSERT_EQUAL(0, cm.getConfig().boardCount);
    TEST_ASSERT_FALSE(cm.hasConfiguredBoards());
    
    // Add an incomplete board (no ID)
    Config& c = cm.getConfig();
    c.boardCount = 1;
    c.boards[0].type = MODE_RAIL;
    c.boards[0].id[0] = '\0';
    TEST_ASSERT_FALSE(cm.hasConfiguredBoards());

    // Add a valid board
    strcpy(c.boards[0].id, "PAD");
    TEST_ASSERT_TRUE(cm.hasConfiguredBoards());
}

void test_v23_to_v24_migration() {
    appContext ctx;
    ConfigManager& cm = ctx.getConfigManager();
    
    // Inject v2.3 legacy config
    const char* v23_config = "{\"version\":2.3,\"turnOffOledInSleep\":true,\"boards\":[{\"type\":3,\"id\":\"CLK\",\"name\":\"Clock\"},{\"type\":1,\"id\":\"PAD\",\"name\":\"Rail\"}]}";
    LittleFS._setFile("/config.json", v23_config);
    
    // Load config
    cm.loadConfig();
    
    const Config& c = cm.getConfig();
    TEST_ASSERT_EQUAL(2, c.boardCount);
    TEST_ASSERT_EQUAL_FLOAT(2.4f, c.configVersion);
    
    // Board 0 (CLK) should have oledOff = true
    TEST_ASSERT_EQUAL(MODE_CLOCK, c.boards[0].type);
    TEST_ASSERT_TRUE(c.boards[0].oledOff);
    
    // Board 1 (RAIL) should have oledOff = false (legacy flag only applies to screensavers)
    TEST_ASSERT_EQUAL(MODE_RAIL, c.boards[1].type);
    TEST_ASSERT_FALSE(c.boards[1].oledOff);
}

void test_save_load_roundtrip() {
    appContext ctx;
    ConfigManager& cm = ctx.getConfigManager();
    
    // Set a config
    Config& c = cm.getConfig();
    strcpy(c.hostname, "TestHost");
    c.brightness = 128;
    c.boardCount = 1;
    c.boards[0].type = MODE_BUS;
    strcpy(c.boards[0].id, "123");
    c.boards[0].oledOff = true;
    
    // Save
    TEST_ASSERT_TRUE(cm.saveConfig());
    
    // Clear and reload
    cm.loadConfig();
    
    // Verify
    const Config& r = cm.getConfig();
    TEST_ASSERT_EQUAL_STRING("TestHost", r.hostname);
    TEST_ASSERT_EQUAL(128, r.brightness);
    TEST_ASSERT_EQUAL(1, r.boardCount);
    TEST_ASSERT_EQUAL(MODE_BUS, r.boards[0].type);
    TEST_ASSERT_EQUAL_STRING("123", r.boards[0].id);
    TEST_ASSERT_TRUE(r.boards[0].oledOff);
}

void runConfigManagerTests() {
    RUN_TEST(test_hasConfiguredBoards);
    RUN_TEST(test_v23_to_v24_migration);
    RUN_TEST(test_save_load_roundtrip);
}
