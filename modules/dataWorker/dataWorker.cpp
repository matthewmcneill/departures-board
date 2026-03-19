/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/dataWorker/dataWorker.cpp
 * Description: Implementation of the centralized FreeRTOS data fetch queue.
 *
 * Exported Functions/Classes:
 * - dataWorker::dataWorker: Constructor.
 * - dataWorker::init: Initializes the FreeRTOS queue and spawns the pinned Core 0 worker task.
 * - dataWorker::enqueueRequest: Submits an iDataSource pointer to the worker queue.
 */

#include "dataWorker.hpp"
#include <logger.hpp>

/**
 * @brief Constructor for the Centralized Data Worker.
 */
dataWorker::dataWorker() : fetchQueue(nullptr), workerTaskHandle(nullptr), debugEnabled(false) {
}

/**
 * @brief Initializes the FreeRTOS queue and spawns the pinned Core 0 worker task.
 * @param enableDebug Optional flag to enable verbose logging for the queue.
 */
void dataWorker::init(bool enableDebug) {
    debugEnabled = enableDebug;
    
    // --- Step 1: Initialize the Queue ---
    // The queue just holds pointers, keeping memory footprint incredibly small
    fetchQueue = xQueueCreate(MAX_QUEUE_ITEMS, sizeof(iDataSource*));
    if (fetchQueue == nullptr) {
        LOG_ERROR("DATA", "DataWorker: Failed to create fetchQueue. FATAL.");
        return;
    }

    // --- Step 2: Spawn Pinned Worker Task ---
    // We pin this to Core 0 to ensure network requests don't interrupt the UI logic on Core 1
    // We give it tskIDLE_PRIORITY + 1 so it yields to more critical hardware tasks
    xTaskCreatePinnedToCore(
        workerTaskLoop, 
        "Data_Worker", 
        8192, 
        this, 
        tskIDLE_PRIORITY + 1, 
        &workerTaskHandle, 
        0
    );
    
    LOG_INFO("DATA", "DataWorker: Centralized Queue initialized on Core 0");
}

/**
 * @brief Submits a data source for background fetching.
 * @param source Pointer to the iDataSource that requires fetching.
 * @return true if the item was successfully queued, false if the queue is full.
 */
bool dataWorker::enqueueRequest(iDataSource* source) {
    if (fetchQueue == nullptr || source == nullptr) return false;

    // Check if the source is already in the queue to avoid duplication.
    // Technically FreeRTOS doesn't have an xQueuePeek matching function for specific pointers,
    // but the individual clients (like weatherClient) are expected to block duplicate submissions 
    // by checking their own state, so we just attempt to queue it here.

    if (xQueueSend(fetchQueue, &source, (TickType_t)10) != pdPASS) {
        LOG_WARN("DATA", "DataWorker: Queue full! Dropping fetch request.");
        return false;
    }
    
    if (debugEnabled) {
        LOG_DEBUG("DATA", "DataWorker: Fetch request queued. Pending items: " + String(uxQueueMessagesWaiting(fetchQueue)));
    }
    
    return true;
}

/**
 * @brief Static FreeRTOS Entry Point for the background worker task.
 *        This loop permanently blocks until an item is pushed to the queue.
 * @param pvParameters Pointer to the executing dataWorker instance.
 */
void dataWorker::workerTaskLoop(void* pvParameters) {
    dataWorker* worker = static_cast<dataWorker*>(pvParameters);
    iDataSource* activeSource;

    while (true) {
        // Block indefinitely until a fetch request pointer arrives in the queue
        if (xQueueReceive(worker->fetchQueue, &activeSource, portMAX_DELAY) == pdPASS) {
            
            if (activeSource != nullptr) {
                if (worker->debugEnabled) {
                    LOG_DEBUG("DATA", "DataWorker: Dequeued request. Executing internal fetch sequence...");
                }
                
                // Execute the blocking payload. Since we are inside the single Core 0 worker task,
                // this strictly serializes all TLS allocations across the system.
                activeSource->executeFetch();
                
                if (worker->debugEnabled) {
                    LOG_DEBUG("DATA", "DataWorker: Execution finished. Ready for next.");
                }
            }
        }
    }
}
