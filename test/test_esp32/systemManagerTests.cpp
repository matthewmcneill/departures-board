#include <unity.h>
#include "appContext.hpp"
#include "systemManager.hpp"
#include "displayManager.hpp"
#include "boards/interfaces/iDisplayBoard.hpp"
#include "WiFi.h"

/**
 * @brief Unit test for SystemManager round-robin scheduling.
 */
void test_system_manager_round_robin() {
    appContext ctx;
    systemManager& sm = ctx.getsystemManager();
    
    // Initialize with mock context
    sm.begin(&ctx);
    
    // Access configuration via mock context
    Config& cfg = ctx.getConfigManager().getConfig();
    
    // Setup 3 boards to test update cycling
    cfg.boardCount = 3;
    cfg.apiRefreshRate = 180000; // 3 minutes
    
    // Force a trigger
    // WiFi mock defaults to WL_CONNECTED
    
    // Execute a tick
    sm.tick();
    
    // Verify that the next update time was correctly scheduled in the future
    uint32_t nextUpdate = sm.getNextRoundRobinUpdate();
    TEST_ASSERT_TRUE(nextUpdate > millis());
}

/**
 * @brief Entry point for the system manager test suite.
 */
void runSystemManagerTests() {
    RUN_TEST(test_system_manager_round_robin);
}
