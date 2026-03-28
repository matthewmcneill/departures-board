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
 * Description: Implementation of the scheduler logic. Evaluates the current system time 
 *              against configured rules and manual overrides.
 *
 * Exported Functions/Classes:
 * - schedulerManager::schedulerManager: Constructor initializes context reference.
 * - schedulerManager::begin: Performs initial rule evaluation and state stabilization.
 * - schedulerManager::triggerManualOverride: Initiates a high-priority display override.
 * - schedulerManager::getActiveBoards: Performs the core time-based schedule evaluation.
 */

#include "schedulerManager.hpp"
#include "appContext.hpp"
#include <Arduino.h>
#include <logger.hpp>
#include <algorithm>

schedulerManager::schedulerManager(appContext* ctx) 
    : context(ctx), isManualOverrideActive(false), overrideTimestamp(0) {
}

/**
 * @brief Initializes the scheduler manager and performs the initial rule sweep.
 */
void schedulerManager::begin() {
    if (!context) return;
    
    const Config& config = context->getConfigManager().getConfig();
    int ruleCount = 0;
    for (int i = 0; i < MAX_SCHEDULE_RULES; i++) {
        if (config.schedules[i].boardIndex >= 0) ruleCount++;
    }

    LOG_INFO("SCHEDULER", String("🔘 [SCHEDULER] Initializing... Loaded ") + String(ruleCount) + " rules.");
    
    // Perform initial evaluation to stabilize state
    lastActiveBoards = getActiveBoards();
}

/**
 * @brief Trigger the manual override state, resetting the idle timeout.
 */
void schedulerManager::triggerManualOverride() {
    isManualOverrideActive = true;
    overrideTimestamp = millis();
    LOG_INFO("SCHEDULER", "🔘 [SCHEDULER] Manual override triggered. Timeout reset.");
}

std::vector<int> schedulerManager::getActiveBoards() {
    std::vector<int> activeSlots;
    if (!context) return activeSlots;

    const Config& config = context->getConfigManager().getConfig();

    // Check if we are currently in a manual override
    if (isManualOverrideActive) {
        if (millis() - overrideTimestamp >= (config.manualOverrideTimeoutSecs * 1000UL)) {
            isManualOverrideActive = false;
            LOG_INFO("SCHEDULER", "🔘 [SCHEDULER] Manual override timeout expired. Returning to schedule.");
        }
    }

    // If manual override is active, ALL board slots (that are complete) are active
    if (isManualOverrideActive) {
        for (int i = 0; i < config.boardCount; ++i) {
            if (config.boards[i].complete) activeSlots.push_back(i);
        }
        return activeSlots;
    }

    // --- Step 1: Initialize Time Context ---
    // Ensure we are working with the absolute latest system time before evaluation
    context->getTimeManager().updateCurrentTime();
    const struct tm& currentTime = context->getTimeManager().getCurrentTime();
    int currentHour = currentTime.tm_hour;
    int currentMinute = currentTime.tm_min;
    int currentSecond = currentTime.tm_sec; // For log precision
    int currentTotalMinutes = currentHour * 60 + currentMinute;

    // --- Step 2: Evaluate Time-Based Rules ---
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

    // --- Step 3: Handle Unscheduled Fallback ---
    // If no valid rules match (or exist), implicitly return ALL configured boards.
    bool fallbackUsed = false;
    if (activeSlots.empty()) {
        fallbackUsed = true;
        for (int i = 0; i < config.boardCount; i++) {
            if (config.boards[i].complete) {
                activeSlots.push_back(i);
            }
        }
    }

    // --- Step 4: Change Detection & Logging ---
        lastActiveBoards = activeSlots;
        
        char timestamp[10];
        snprintf(timestamp, sizeof(timestamp), "%02d:%02d:%02d", currentHour, currentMinute, currentSecond);

        if (fallbackUsed) {
            LOG_INFO("SCHEDULER", String("🔘 [SCHEDULER] [") + timestamp + "] No active rules! Entering EMPTY FALLBACK (All boards active)");
        } else {
            String boardList = "";
            for (size_t i = 0; i < activeSlots.size(); ++i) {
                if (i > 0) boardList += ", ";
                boardList += String(activeSlots[i]) + ": " + String(config.boards[activeSlots[i]].name);
            }
            LOG_INFO("SCHEDULER", String("🔘 [SCHEDULER] [") + timestamp + "] Active boards updated: [" + boardList + "]");
        }
    }

    return activeSlots;
}
