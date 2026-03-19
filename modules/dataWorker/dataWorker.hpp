/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/dataWorker/dataWorker.hpp
 * Description: Centralized FreeRTOS queue worker for coordinating background network requests.
 *              By forcing data sources to execute sequentially on Core 0, this module
 *              prevents concurrent TLS allocations, protecting the heap from exhaustion
 *              and preventing Watchdog Timer (WDT) panics.
 *
 * Exported Functions/Classes:
 * - dataWorker: Singleton worker class managing the FreeRTOS Queue and Task.
 *   - init(): Initializes the FreeRTOS queue and spawns the pinned Core 0 worker task.
 *   - enqueueRequest(): Submits an iDataSource pointer to the worker queue.
 */

#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include "../displayManager/boards/interfaces/iDataSource.hpp"

#define MAX_QUEUE_ITEMS 10 // Maximum pending fetch requests

/**
 * @brief Singleton worker class managing the background data fetching queue.
 */
class dataWorker {
public:
    dataWorker();
    
    /**
     * @brief Initializes the FreeRTOS queue and spawns the pinned Core 0 worker task.
     * @param enableDebug Optional flag to enable verbose logging for the queue.
     */
    void init(bool enableDebug = false);

    /**
     * @brief Submits a data source for background fetching.
     * @param source Pointer to the iDataSource that requires fetching.
     * @return true if the item was successfully queued, false if the queue is full.
     */
    bool enqueueRequest(iDataSource* source);

private:
    QueueHandle_t fetchQueue;   // Standard FreeRTOS queue holding iDataSource pointers
    TaskHandle_t workerTaskHandle;
    bool debugEnabled;

    /**
     * @brief Static FreeRTOS Entry Point for the background worker task.
     * @param pvParameters Pointer to the executing dataWorker instance.
     */
    static void workerTaskLoop(void* pvParameters);
};

