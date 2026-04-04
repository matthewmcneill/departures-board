/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/boards/nationalRailBoard/nrRDMDataProvider.hpp
 * Description: JSON-based data provider interface implementation for Rail Data Marketplace API.
 *
 * Exported Functions/Classes:
 * - nrRDMDataProvider: Concrete RDM backend provider implementing iNationalRailDataProvider.
 *   - updateData(): Intercept and request a priority update from the data manager.
 *   - getLastUpdateStatus(): Returns the final execution status.
 *   - getLastErrorMsg(): Returns text description of last failure.
 *   - testConnection(): Executes a miniature payload request for API key validation.
 *   - executeFetch(): Background HTTP engine for retrieving and parsing RDM JSON payload.
 *   - configure(): Bootstrapping routine to sink configuration parameters into instance.
 */

#pragma once

#include "iNationalRailDataProvider.hpp"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <memory>

/**
 * @brief Concrete JSON implementation of iNationalRailDataProvider for the modern RDM API.
 */
class nrRDMDataProvider : public iNationalRailDataProvider {
private:
    std::unique_ptr<NationalRailStation> stationData; // Internal background writing buffer
    std::unique_ptr<NationalRailStation> renderData; // Thread-safe protected readable payload
    MessagePool messagesData; // Mutable string pool for alerts
    MessagePool renderMessages; // Immutable presented pool

    SemaphoreHandle_t dataMutex; // Mutex protecting the renderable object swap
    volatile UpdateStatus taskStatus; // Current background state enum
    char lastErrorMessage[128]; // Detailed string descriptor of errors

    // Configuration Settings
    char rdmToken[128];
    char crsCode[4];
    char callingCrsCode[4];
    char platformFilter[54];
    int nrTimeOffset;
    uint32_t nextFetchTimeMillis; // Real-world millis representation to delay API polling

    /**
     * @brief Raw parsing engine. Scans payload and populates stationData.
     * @param payload JSON response payload from the RDM backend.
     * @return True if parsing completed without critical structure loss.
     */
    bool parseRDMJson(const char* payload);

public:
    nrRDMDataProvider();
    virtual ~nrRDMDataProvider();

    // --- iDataSource overrides ---

    /**
     * @brief Request an asynchronous priority background update loop.
     * @return Status marker reflecting request queued logic.
     */
    UpdateStatus updateData() override;
    
    UpdateStatus getLastUpdateStatus() const override { return taskStatus; }
    
    const char* getLastErrorMsg() const override { return lastErrorMessage; }
    
    /**
     * @brief Returns the strict legal attribution string for the RDM Gateway.
     * @return Literal static string for the OLED message widget.
     */
    const char* getAttributionString() const override { return "Powered by Rail Delivery Group"; }
    
    /**
     * @brief Ping the RDM backend with minimal footprint for authorization test.
     * @param token Explicit HTTP Key token.
     * @param stationId Explicit Station identifier (CRS).
     * @return UpdateStatus evaluation determining if logic can read the stream.
     */
    UpdateStatus testConnection(const char* token = nullptr, const char* stationId = nullptr) override;
    
    uint32_t getNextFetchTime() override { return nextFetchTimeMillis; }
    PriorityTier getPriorityTier() override { return PriorityTier::PRIO_HIGH; }
    void setNextFetchTime(uint32_t forceTimeMillis) override { nextFetchTimeMillis = forceTimeMillis; }

    void lockData() override { if(dataMutex) xSemaphoreTake(dataMutex, portMAX_DELAY); }
    void unlockData() override { if(dataMutex) xSemaphoreGive(dataMutex); }
    
    /**
     * @brief Heavy background polling loop retrieving REST/JSON. Should trace within thread.
     */
    void executeFetch() override;

    // --- iNationalRailDataProvider overrides ---

    void configure(const char* token, const char* crs, const char* filter = "", const char* callingCrs = "", int offset = 0) override;
    NationalRailStation* getStationData() override { return renderData.get(); }
    MessagePool* getMessagesData() override { return &renderMessages; }
};
