/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/schedulerManager/schedulerManager.cpp
 * Description: Implementation of the scheduler logic.
 */

#include "schedulerManager.hpp"
#include "appContext.hpp"
#include <Arduino.h>
#include <logger.hpp>
#include <algorithm>

schedulerManager::schedulerManager(appContext* ctx) 
    : context(ctx), isManualOverrideActive(false), overrideTimestamp(0) {
}

void schedulerManager::triggerManualOverride() {
    isManualOverrideActive = true;
    overrideTimestamp = millis();
    LOG_INFO("SCHEDULER", "Manual override triggered. Timeout reset.");
}

std::vector<int> schedulerManager::getActiveBoards() {
    std::vector<int> activeSlots;
    if (!context) return activeSlots;

    const Config& config = context->getConfigManager().getConfig();

    // Check if we are currently in a manual override
    if (isManualOverrideActive) {
        if (millis() - overrideTimestamp >= (config.manualOverrideTimeoutSecs * 1000UL)) {
            isManualOverrideActive = false;
            LOG_INFO("SCHEDULER", "Manual override timeout expired. Returning to schedule.");
        }
    }

    // If manual override is active, ALL board slots (that are complete) are active
    if (isManualOverrideActive) {
        for (int i = 0; i < config.boardCount; ++i) {
            if (config.boards[i].complete) activeSlots.push_back(i);
        }
        return activeSlots;
    }

    // Otherwise, evaluate exactly time-based rules
    const struct tm& currentTime = context->getTimeManager().getCurrentTime();
    int currentHour = currentTime.tm_hour;
    int currentMinute = currentTime.tm_min;
    int currentTotalMinutes = currentHour * 60 + currentMinute;

    // Evaluate scheduled rules
    for (int i = 0; i < MAX_SCHEDULE_RULES; i++) {
        const ScheduleRule& rule = config.schedules[i];
        if (rule.boardIndex >= 0 && rule.boardIndex < config.boardCount) {
            // Robustness check: is the targeted board actually configured?
            if (config.boards[rule.boardIndex].complete) {
                int startTotalMinutes = rule.startHour * 60 + rule.startMinute;
                int endTotalMinutes = rule.endHour * 60 + rule.endMinute;
                
                // Handle rules that wrap around midnight (e.g., 23:00 to 07:00)
                bool isActive = false;
                if (startTotalMinutes <= endTotalMinutes) {
                    isActive = (currentTotalMinutes >= startTotalMinutes && currentTotalMinutes <= endTotalMinutes);
                } else {
                    isActive = (currentTotalMinutes >= startTotalMinutes || currentTotalMinutes <= endTotalMinutes);
                }

                if (isActive) {
                    // Check for duplicates before adding
                    if (std::find(activeSlots.begin(), activeSlots.end(), rule.boardIndex) == activeSlots.end()) {
                        activeSlots.push_back(rule.boardIndex);
                    }
                }
            }
        }
    }

    // Empty Schedule Fallback logic: if no valid rules match (or exist), implicitly return ALL configured boards.
    if (activeSlots.empty()) {
        for (int i = 0; i < config.boardCount; i++) {
            if (config.boards[i].complete) {
                activeSlots.push_back(i);
            }
        }
    }

    return activeSlots;
}
