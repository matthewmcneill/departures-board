/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/dataManager/dataManager.hpp
 * Description: Centralized FreeRTOS queue manager for transport data fetching.
 *
 * Exported Functions/Classes:
 * - dataManager: [Class] Core 0 background worker coordinator.
 *   - init(): Task spawning entry point.
 *   - registerSource(): Injects into the polling registry.
 *   - requestPriorityFetch(): Triggers immediate out-of-band updates.
 *   - unregisterSource(): Thread-safe removal of data providers.
 */

#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <vector>
#include "iDataSource.hpp"

/**
 * @brief Singleton manager class handling the dynamic schedule-driven background fetch queue.
 */
class dataManager {
public:
    dataManager();
    
     /**
     * @brief Initializes the FreeRTOS event queue and spawns the pinned Core 0 worker task.
     */
    void init();

    /**
     * @brief Registers a data source with the manager.
     */
    void registerSource(iDataSource* source);

    /**
     * @brief Sends a wake event to immediately evaluate scheduling (e.g., after priority change).
     */
    void requestPriorityFetch(iDataSource* source);

    /**
     * @brief Formally removes a source from the background registry.
     *        If the source is currently being fetched, this will block 
     *        unitl the fetch is complete to prevent use-after-free.
     */
    void unregisterSource(iDataSource* source);

    // --- Migrated Polling State ---
    unsigned long getNextRoundRobinUpdate() const { return nextRoundRobinUpdate; }
    void setNextRoundRobinUpdate(unsigned long time) { nextRoundRobinUpdate = time; }
    
    unsigned long getLastDataLoadTime() const { return lastDataLoadTime; }
    bool getNoDataLoaded() const { return noDataLoaded; }
    void setNoDataLoaded(bool val) { noDataLoaded = val; }
    
    int getDataLoadSuccess() const { return dataLoadSuccess; }
    int getDataLoadFailure() const { return dataLoadFailure; }
    unsigned long getLastLoadFailure() const { return lastLoadFailure; }
    UpdateStatus getLastUpdateResult() const { return lastUpdateResult; }
    void setLastUpdateResult(UpdateStatus status) { lastUpdateResult = status; }
    
    int getBackgroundUpdateIndex() const { return backgroundUpdateIndex; }
    void setBackgroundUpdateIndex(int index) { backgroundUpdateIndex = index; }
    
    int getLastActiveSlotIndex() const { return lastActiveSlotIndex; }
    void setLastActiveSlotIndex(int index) { lastActiveSlotIndex = index; }

    void incrementSuccess() { dataLoadSuccess++; }
    void incrementFailure() { dataLoadFailure++; lastLoadFailure = millis(); }

private:
    std::vector<iDataSource*> registry;
    SemaphoreHandle_t registryMutex;  // Protects the registry vector across cores
    volatile iDataSource* currentlyExecuting; // Tracks the active fetch target
    QueueHandle_t priorityEventQueue; // FreeRTOS queue holding priority events
    TaskHandle_t workerTaskHandle;

    // --- Polling State Variables ---
    unsigned long nextRoundRobinUpdate;
    int backgroundUpdateIndex;
    unsigned long lastDataLoadTime;
    bool noDataLoaded;
    int dataLoadSuccess;
    int dataLoadFailure;
    unsigned long lastLoadFailure;
    UpdateStatus lastUpdateResult;
    int lastActiveSlotIndex;

    /**
     * @brief Static FreeRTOS Entry Point for the dynamic scheduling loop.
     * @param pvParameters Pointer to the executing dataManager instance.
     */
    static void workerTaskLoop(void* pvParameters);
};
