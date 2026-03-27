/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/dataManager/dataManager.cpp
 * Description: Implementation of the centralized, priority-aware FreeRTOS data fetch manager.
 */

#include "dataManager.hpp"
#include <logger.hpp>

/**
 * @brief Constructor for the Centralized Data Manager.
 */
dataManager::dataManager() : priorityEventQueue(nullptr), workerTaskHandle(nullptr), debugEnabled(false) {
}

/**
 * @brief Initializes the FreeRTOS queue and spawns the pinned Core 0 worker task.
 * @param enableDebug Optional flag to enable verbose logging for the queue.
 */
void dataManager::init(bool enableDebug) {
    debugEnabled = enableDebug;
    
    // We only need a tiny queue to wake the worker up early if a priority event occurs
    priorityEventQueue = xQueueCreate(5, sizeof(iDataSource*));
    if (priorityEventQueue == nullptr) {
        LOG_ERROR("DATA", "DataManager: Failed to create priorityEventQueue. FATAL.");
        return;
    }

    // --- Step 2: Spawn Pinned Worker Task ---
    // We pin this to Core 0 to ensure network requests don't interrupt the UI logic on Core 1
    // We give it tskIDLE_PRIORITY + 1 so it yields to more critical hardware tasks
    xTaskCreatePinnedToCore(
        workerTaskLoop, 
        "Data_Manager", 
        8192, 
        this, 
        tskIDLE_PRIORITY + 1, 
        &workerTaskHandle, 
        0
    );
    
    LOG_INFO("DATA", "DataManager: Schedule-Driven Loop initialized on Core 0");
}

void dataManager::registerSource(iDataSource* source) {
    if (source != nullptr) {
        registry.push_back(source);
        if (debugEnabled) {
            LOG_DEBUG("DATA", "DataManager: Source registered. Total: " + String(registry.size()));
        }
    }
}

/**
 * @brief Submits a priority wake event.
 */
void dataManager::requestPriorityFetch(iDataSource* source) {
    if (priorityEventQueue == nullptr) return;
    
    if (source != nullptr) {
        // Force the source's fetch time to now to guarantee it gets picked up immediately
        source->setNextFetchTime(millis());
    }
    
    // Send a wake-up event to unblock xQueueReceive instantly
    if (xQueueSendToFront(priorityEventQueue, &source, (TickType_t)10) != pdPASS) {
        LOG_WARN("DATA", "DataManager: Priority Queue full! Wake event dropped.");
    } else {
        if (debugEnabled) {
            LOG_DEBUG("DATA", "DataManager: Priority wake event dispatched.");
        }
    }
}

/**
 * @brief Static FreeRTOS Entry Point for the dynamic scheduling loop.
 */
void dataManager::workerTaskLoop(void* pvParameters) {
    dataManager* manager = static_cast<dataManager*>(pvParameters);
    iDataSource* eventSource;

    while (true) {
        uint32_t now = millis();
        uint32_t soonestTime = now + 60000; // Default max sleep 1 minute
        iDataSource* bestSource = nullptr;
        uint8_t bestTier = 255;

        // Iterate registry to find the most pressing source
        for (iDataSource* src : manager->registry) {
            uint32_t fetchTime = src->getNextFetchTime();
            uint8_t tier = src->getPriorityTier();
            
            if (now >= fetchTime) {
                // It's due (or overdue). Priority takes precedence over time.
                if (tier < bestTier) {
                    bestTier = tier;
                    bestSource = src;
                }
            } else {
                // Not due yet. Track the soonest time so we sleep accurately.
                if (fetchTime < soonestTime) {
                    soonestTime = fetchTime;
                }
            }
        }

        // Determine block time based on registry sweep
        TickType_t ticksToWait = 0;
        if (bestSource == nullptr && soonestTime > now) {
            ticksToWait = pdMS_TO_TICKS(soonestTime - now);
        }

        if (ticksToWait > 0 && manager->debugEnabled) {
            LOG_DEBUG("DATA", "DataManager: Sleeping for " + String(soonestTime - now) + "ms");
        }

        iDataSource* targetToExecute = nullptr;

        // Block on the priority queue, OR poll it if ticksToWait == 0
        if (xQueueReceive(manager->priorityEventQueue, &eventSource, ticksToWait) == pdPASS) {
            if (manager->debugEnabled && ticksToWait > 0) {
                LOG_INFO("DATA", "DataManager: Woken early by priority event override.");
            }
            if (eventSource != nullptr) {
                targetToExecute = eventSource;
            }
        } else {
            // Queue was empty (timed out). Fallback to the registry's best source if we had one.
            if (bestSource != nullptr) {
                targetToExecute = bestSource;
            }
        }

        // Single execution dispatch path
        if (targetToExecute != nullptr) {
            LOG_INFO("DATA", "DataManager: Executing fetch for tier " + String(targetToExecute->getPriorityTier()));
            
            targetToExecute->executeFetch();
            
            // SAFETY NET: If the source failed to natively update its nextFetchTime 
            // (e.g. returning early on HTTP error), forcefully back it off to prevent
            // an infinite tight loop that starves the Wi-Fi task and triggers TWDT.
            if (targetToExecute->getNextFetchTime() <= millis()) {
                LOG_WARN("DATA", "DataManager: Source failed to update schedule! Forcing 15s backoff.");
                targetToExecute->setNextFetchTime(millis() + 15000);
            }
        }
    }
}
