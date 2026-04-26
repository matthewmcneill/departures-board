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
 *              determine which display boards should be active.
 *
 * Exported Functions/Classes:
 * - schedulerManager: [Class] Core manager for time-based display scheduling.
 *   - begin(): Initializes manager state.
 *   - triggerManualOverride(): Wakes system and resets idle timeout.
 *   - getActiveBoards(): [Eval] Returns the effective list of allowed board indices.
 */

#pragma once

#include <vector>

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
     * @brief Initializes the manager state and performs initial rule evaluation logging.
     */
    void begin();

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
    std::vector<int> lastActiveBoards; ///< Previously active board list for logging
};
