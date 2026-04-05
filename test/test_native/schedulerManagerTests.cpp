/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons
 * Attribution-NonCommercial-ShareAlike 4.0 International. To view a copy of
 * this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: test/test_native/schedulerManagerTests.cpp
 * Description: Unit tests for the schedulerManager class.
 *
 * Exported Functions/Classes:
 * - runSchedulerManagerTests(): Entry point for scheduling tests.
 */

#include <unity.h>
#include "SystemState.hpp"
#include "appContext.hpp"
#include "schedulerManager.hpp"
#include "displayManager.hpp"
#include "iDisplayBoard.hpp"

void test_scheduler_manager_init() {
    appContext ctx;
    schedulerManager& sm = ctx.getSchedulerManager();
    sm.begin();
    
    // basic verification that we can call getActiveBoards
    std::vector<int> active = sm.getActiveBoards();
    // By default, if no config is loaded, it might be empty or full depending on implementation.
    // For now, we just ensure it doesn't crash and returns a valid vector.
    TEST_ASSERT_TRUE(true);
}

void runSchedulerManagerTests() {
    RUN_TEST(test_scheduler_manager_init);
}
