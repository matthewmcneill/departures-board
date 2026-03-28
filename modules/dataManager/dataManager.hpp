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
 * Description: Centralized FreeRTOS queue manager for coordinating background network requests.
 *              By forcing data sources to execute sequentially on Core 0, this module
 *              prevents concurrent TLS allocations, protecting the heap from exhaustion
 *              and preventing Watchdog Timer (WDT) panics.
 *
 * Exported Functions/Classes:
 * - dataManager: Singleton manager class.
 *   - init(): Initializes the FreeRTOS event queue and spawns the pinned Core 0 worker task.
 *   - registerSource(): Registers an iDataSource logic element to the background queueing loop.
 *   - requestPriorityFetch(): Submits a priority wake request.
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
     * @param enableDebug Optional flag to enable verbose logging.
     */
    void init(bool enableDebug = false);

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

private:
    std::vector<iDataSource*> registry;
    SemaphoreHandle_t registryMutex;  // Protects the registry vector across cores
    volatile iDataSource* currentlyExecuting; // Tracks the active fetch target
    QueueHandle_t priorityEventQueue; // FreeRTOS queue holding priority events
    TaskHandle_t workerTaskHandle;
    bool debugEnabled;

    /**
     * @brief Static FreeRTOS Entry Point for the dynamic scheduling loop.
     * @param pvParameters Pointer to the executing dataManager instance.
     */
    static void workerTaskLoop(void* pvParameters);
};
