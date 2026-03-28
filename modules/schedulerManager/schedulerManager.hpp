/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/schedulerManager/schedulerManager.hpp
 * Description: Evaluates the system schedule rules against current time to 
 *              determine which display boards should be active. Handles manual 
 *              override button states.
 *
 * Exported Functions/Classes:
 * - schedulerManager: The class handling schedule lifecycle and overrides.
 */

#pragma once

#include <vector>
#include "configManager.hpp"

// Forward declaration
class appContext;

class schedulerManager {
public:
    /**
     * @brief Constructs the scheduler manager.
     * @param ctx Pointer to the global application context.
     */
    schedulerManager(appContext* ctx);

    /**
     * @brief Trigger the manual override state, resetting the idle timeout.
     */
    void triggerManualOverride();

    /**
     * @brief Evaluates the current time against the configured schedules.
     * @return A vector of board indices that are currently allowed to be displayed.
     */
    std::vector<int> getActiveBoards();

private:
    appContext* context;              ///< Application context reference
    bool isManualOverrideActive;      ///< Flag for button override
    unsigned long overrideTimestamp;  ///< Idle tracker for override timeout
};
