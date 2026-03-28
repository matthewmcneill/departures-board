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
dataManager::dataManager() : priorityEventQueue(nullptr), registryMutex(nullptr), currentlyExecuting(nullptr), workerTaskHandle(nullptr), debugEnabled(false) {
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

    registryMutex = xSemaphoreCreateBinary();
    if (registryMutex == nullptr) {
        LOG_ERROR("DATA", "DataManager: Failed to create registryMutex. FATAL.");
        return;
    }
    xSemaphoreGive(registryMutex);

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
        if (registryMutex && xSemaphoreTake(registryMutex, portMAX_DELAY) == pdPASS) {
            registry.push_back(source);
            xSemaphoreGive(registryMutex);
            if (debugEnabled) {
                LOG_DEBUG("DATA", "DataManager: Source registered. Total: " + String(registry.size()));
            }
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
    
    // Wake up the worker task safely
    if (xQueueSendToFront(priorityEventQueue, &source, (TickType_t)10) != pdPASS) {
        LOG_WARN("DATA", "DataManager: Priority Queue full! Wake event dropped.");
    } else if (debugEnabled) {
        LOG_DEBUG("DATA", "DataManager: Priority wake event dispatched.");
    }
}

void dataManager::unregisterSource(iDataSource* source) {
    if (source == nullptr || registryMutex == nullptr) return;

    LOG_INFO("DATA", "DataManager: Unregistering source...");
    
    if (xSemaphoreTake(registryMutex, portMAX_DELAY) == pdPASS) {
        auto it = std::find(registry.begin(), registry.end(), source);
        if (it != registry.end()) {
            registry.erase(it);
            if (debugEnabled) {
                LOG_DEBUG("DATA", "DataManager: Source removed from registry.");
            }
        }
        xSemaphoreGive(registryMutex);
    }

    // BLOCKING SAFETY: If this source is currently being executed on Core 0,
    // we MUST block the caller (typically a board destructor on Core 1) 
    // until the fetch completes. Destroying the object mid-fetch is what causes the crash.
    int timeout = 0;
    while (currentlyExecuting == source && timeout++ < 1000) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    if (timeout >= 1000) {
        LOG_ERROR("DATA", "DataManager: Timeout waiting for active fetch to conclude during unregistration!");
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
        if (manager->registryMutex && xSemaphoreTake(manager->registryMutex, portMAX_DELAY) == pdPASS) {
            for (iDataSource* src : manager->registry) {
                uint32_t fetchTime = src->getNextFetchTime();
                uint8_t tier = src->getPriorityTier();
                
                if (now >= fetchTime) {
                    if (tier < bestTier) {
                        bestTier = tier;
                        bestSource = src;
                    }
                } else {
                    if (fetchTime < soonestTime) soonestTime = fetchTime;
                }
            }
            // Temporarily set currentlyExecuting while still inside Mutex to ensure unregisterSource waits
            if (bestSource != nullptr) manager->currentlyExecuting = bestSource;
            xSemaphoreGive(manager->registryMutex);
        }

        // Determine block time
        TickType_t ticksToWait = 0;
        if (bestSource == nullptr && soonestTime > now) {
            ticksToWait = pdMS_TO_TICKS(soonestTime - now);
        }

        iDataSource* targetToExecute = nullptr;

        // Block on the priority queue
        if (xQueueReceive(manager->priorityEventQueue, &eventSource, ticksToWait) == pdPASS) {
            if (manager->registryMutex && xSemaphoreTake(manager->registryMutex, portMAX_DELAY) == pdPASS) {
                if (eventSource != nullptr) {
                    targetToExecute = eventSource;
                    manager->currentlyExecuting = targetToExecute;
                } else if (bestSource != nullptr) {
                    targetToExecute = bestSource;
                    manager->currentlyExecuting = targetToExecute;
                }
                xSemaphoreGive(manager->registryMutex);
            }
        } else {
            targetToExecute = bestSource;
        }

        if (targetToExecute != nullptr) {
            LOG_INFO("DATA", "DataManager: Executing fetch for tier " + String(targetToExecute->getPriorityTier()));
            
            targetToExecute->executeFetch();
            
            // SAFETY NET: Ensure nextFetchTime is always advanced to prevent tight-looping
            if (targetToExecute->getNextFetchTime() <= millis()) {
                LOG_WARN("DATA", "DataManager: Source failed to update schedule! Forcing 15s backoff.");
                targetToExecute->setNextFetchTime(millis() + 15000);
            }

            // Finalize execution state safely WHILE targetToExecute is still guaranteed valid
            manager->currentlyExecuting = nullptr;
        }
        yield();
    }
}
