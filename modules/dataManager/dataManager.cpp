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
 *
 * Exported Functions/Classes:
 * - dataManager: [Class implementation]
 *   - init: Initializes FreeRTOS concurrency primitives and starts the worker.
 *   - registerSource/unregisterSource: Registry management for data providers.
 *   - requestPriorityFetch: High-priority out-of-band fetch trigger.
 *   - workerTaskLoop: [Static] The background fetch orchestration loop on Core 0.
 */

#include "dataManager.hpp"
#include <logger.hpp>
#include <appContext.hpp>

/**
 * @brief Constructor for the Centralized Data Manager.
 */
/**
 * @brief Construct a new dataManager.
 * Initializes all polling and error tracking state to defaults.
 */
dataManager::dataManager() : 
    priorityEventQueue(nullptr), 
    registryMutex(nullptr), 
    currentlyExecuting(nullptr), 
    workerTaskHandle(nullptr),
    nextRoundRobinUpdate(0),
    backgroundUpdateIndex(0),
    lastDataLoadTime(0),
    noDataLoaded(true),
    dataLoadSuccess(0),
    dataLoadFailure(0),
    lastLoadFailure(0),
    lastUpdateResult(UpdateStatus::SUCCESS),
    lastActiveSlotIndex(0) {
}

/**
 * @brief Initializes the FreeRTOS event queue and spawns the pinned Core 0 worker task.
 * Sets up binary semaphores for thread-safe registry access.
 */
void dataManager::init(class appContext* ctx) {
    this->context = ctx;
    
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

/**
 * @brief Register a data source for background polling.
 * @param source Pointer to iDataSource implementation.
 */
void dataManager::registerSource(iDataSource* source) {
    if (source != nullptr) {
        if (registryMutex && xSemaphoreTake(registryMutex, portMAX_DELAY) == pdPASS) {
            registry.push_back(source);
            xSemaphoreGive(registryMutex);
            LOG_VERBOSEf("DATA", "DataManager: Source registered. Total: %d", (int)registry.size());
        }
    }
}

void dataManager::serializeAllSources(JsonObject& out) {
    if (registryMutex && xSemaphoreTake(registryMutex, pdMS_TO_TICKS(100)) == pdPASS) {
        JsonArray sources = out["sources"].to<JsonArray>();
        for (auto* source : registry) {
            JsonObject sourceObj = sources.add<JsonObject>();
            sourceObj["name"] = source->getAttributionString();
            sourceObj["status"] = static_cast<int>(getLastUpdateResult());
            
            JsonObject data = sourceObj["data"].to<JsonObject>();
            source->serializeData(data);
        }
        xSemaphoreGive(registryMutex);
    } else {
        out["error"] = "Registry locked";
    }
}

/**
 * @brief Submits a priority wake event to the background worker.
 * Guarantees the source will be evaluated for immediate fetch in the next cycle.
 * @param source Pointer to the iDataSource requesting the priority wake.
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
    } else {
        LOG_VERBOSE("DATA", "DataManager: Priority wake event dispatched.");
    }
}

/**
 * @brief Safely remove a source from background polling.
 * Blocks if the source is currently mid-fetch to prevent use-after-free.
 * @param source Pointer to the source to remove.
 */
void dataManager::unregisterSource(iDataSource* source) {
    if (source == nullptr || registryMutex == nullptr) return;

    LOG_INFO("DATA", "DataManager: Unregistering source...");
    
    if (xSemaphoreTake(registryMutex, portMAX_DELAY) == pdPASS) {
        auto it = std::find(registry.begin(), registry.end(), source);
        if (it != registry.end()) {
            registry.erase(it);
            LOG_VERBOSE("DATA", "DataManager: Source removed from registry.");
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
 * Implements the core round-robin and priority-aware fetch orchestration.
 * @param pvParameters Pointer to the executing dataManager instance.
 */
void dataManager::workerTaskLoop(void* pvParameters) {
    dataManager* manager = static_cast<dataManager*>(pvParameters);
    iDataSource* eventSource;

    while (true) {
        uint32_t now = millis();
        uint32_t soonestTime = now + 60000; // Default max sleep 1 minute
        iDataSource* bestSource = nullptr;
        PriorityTier bestTier = static_cast<PriorityTier>(255);

        // Iterate registry to find the most pressing source
        if (manager->registryMutex && xSemaphoreTake(manager->registryMutex, portMAX_DELAY) == pdPASS) {
            for (iDataSource* src : manager->registry) {
                uint32_t fetchTime = src->getNextFetchTime();
                PriorityTier tier = src->getPriorityTier();
                
                if (now >= fetchTime) {
                    if (static_cast<uint8_t>(tier) < static_cast<uint8_t>(bestTier)) {
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
                    // SECURITY: Ensure the queued source hasn't been unregistered/destroyed
                    auto it = std::find(manager->registry.begin(), manager->registry.end(), eventSource);
                    if (it != manager->registry.end()) {
                        targetToExecute = eventSource;
                        manager->currentlyExecuting = targetToExecute;
                    } else {
                        LOG_WARN("DATA", "DataManager: Discarded orphaned queue event.");
                        // Fallback if the priority event was a ghost
                        if (bestSource != nullptr) {
                            targetToExecute = bestSource;
                            manager->currentlyExecuting = targetToExecute;
                        }
                    }
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
            
            AppState state = manager->context ? manager->context->getAppState() : AppState::BOOTING;
            
            if (state == AppState::WIFI_SETUP) {
                // Absolute network blockade in AP Mode: No internet exists, DNS requests will lock LwIP.
                targetToExecute->setNextFetchTime(millis() + 15000);
                if (manager->registryMutex && xSemaphoreTake(manager->registryMutex, portMAX_DELAY) == pdPASS) {
                    manager->currentlyExecuting = nullptr;
                    xSemaphoreGive(manager->registryMutex);
                }
                continue; // Skip execute
            } else if (state != AppState::RUNNING && !(state == AppState::BOOTING && manager->context && manager->context->getFirstLoad()) && targetToExecute->getPriorityTier() != PriorityTier::PRIO_CRITICAL) {
                // Board setup mode has internet, but block routine fetches until UI finishes adding displays.
                targetToExecute->setNextFetchTime(millis() + 15000);
                if (manager->registryMutex && xSemaphoreTake(manager->registryMutex, portMAX_DELAY) == pdPASS) {
                    manager->currentlyExecuting = nullptr;
                    xSemaphoreGive(manager->registryMutex);
                }
                continue; // Skip execute
            }
            
            LOG_INFOf("DATA", "DataManager: Executing fetch for tier %d", (int)targetToExecute->getPriorityTier());
            
            targetToExecute->executeFetch();
            
            // Mark the system as having received at least some hardware data
            if (manager->noDataLoaded) {
                manager->noDataLoaded = false;
            }
            
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
