/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/boards/tflBoard/tflDataSource.hpp
 * Description: TfL data source implementing iDataSource.
 *
 * Exported Functions/Classes:
 * - tflDataSource: [Class] Data client for TfL Unified API.
 *   - updateData(): Initiates JSON fetch.
 *   - getLastUpdateStatus(): Retrieval of fetch result codes.
 *   - getLastErrorMsg(): Accessor for source error strings.
 *   - testConnection(): Validates API credentials and station IDs.
 *   - getNextFetchTime() / setNextFetchTime(): Polling interval management.
 *   - getPriorityTier(): Context-aware fetch prioritization.
 *   - configure(): Sets Naptan ID and optional API key.
 *   - setFilter(): Applies line-specific filtering.
 *   - setDirectionFilter(): Applies Inbound/Outbound filtering.
 *   - setResultLimit(): Maximum arrivals to fetch.
 *   - setTestMode(): Enable lightweight auth-only validation mode.
 *   - getStationData(): Accessor for parsed station metadata.
 *   - getMessagesData(): Accessor for line disruption messages.
 *   - executeFetch(): Internal synchronous HTTPS pipeline.
 * - TflService: [Struct] Single service arrival record.
 * - TflStation: [Struct] Station departure board model.
 */

#ifndef TFL_DATA_SOURCE_HPP
#define TFL_DATA_SOURCE_HPP

#include "../../../dataManager/iDataSource.hpp"
#include "JsonListener.h"
#include "JsonStreamingParser.h"
#include "../../messaging/messagePool.hpp"
#include <memory>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <Arduino.h>

/**
 * @brief Data Length Constants
 */
constexpr size_t TFL_MAX_LOCATION = 45;
constexpr size_t TFL_MAX_LINE = 20;
constexpr size_t TFL_MAX_SERVICES = 9;
constexpr size_t TFL_MAX_FETCH = 20;

/**
 * @brief Data structure for a single London Underground service.
 */
struct TflService {
    const char* orderNum; // Stable pointer to position number (1, 2, 3...)
    char destination[TFL_MAX_LOCATION];
    char lineName[TFL_MAX_LOCATION]; 
    char expectedTime[11];
    int timeToStation; 
};

/**
 * @brief Container for TfL station departure data.
 */
struct TflStation {
    char location[TFL_MAX_LOCATION];
    int numServices;
    bool boardChanged;
    TflService service[TFL_MAX_FETCH];
    uint32_t contentHash;
};



class tflDataSource : public iDataSource, public JsonListener {
private:
    std::unique_ptr<TflStation> stationData; // Background parse buffer for Double Buffering
    std::unique_ptr<TflStation> renderData;  // Active UI buffer containing validated results
    MessagePool messagesData; // Local message pool for background parsing
    MessagePool renderMessages; // Safe local copy for UI rendering

    SemaphoreHandle_t dataMutex; // Thread-safe lock protecting data transfers
    volatile UpdateStatus taskStatus; // Cross-thread execution status tracking (e.g., UpdateStatus::PENDING)

    char lastErrorMsg[128];

    uint32_t nextFetchTimeMillis;
    static const uint32_t BASELINE_MIN_INTERVAL = 30000;

    // API Details
    const char* apiHost = "api.tfl.gov.uk";
    String currentKey;
    String currentObject;
    int id; // Current index for parsing
    bool maxServicesRead; // Flag to prevent multiple services parsing in one pass
    bool isTestMode;      // When true, perform lightweight auth check only
    int maxResults;       // User-defined limit for arrivals response size

    // Configuration
    char tflAppkey[50];
    char tubeId[13];
    char lineFilter[32];
    int directionFilter = 0; // 0: Any, 1: Inbound, 2: Outbound

    // Internal Utilities
    bool pruneFromPhrase(char* input, const char* target);
    void replaceWord(char* input, const char* target, const char* replacement);
    static bool compareTimes(const TflService& a, const TflService& b);
    static const char* serviceNumbers[TFL_MAX_FETCH]; ///< Stable pointers for zero-copy numbering ("1", "2", ...)

public:
    tflDataSource();
    virtual ~tflDataSource() = default;

    // iDataSource Implementation
    UpdateStatus updateData() override;
    UpdateStatus getLastUpdateStatus() const { return taskStatus; }
    const char* getLastErrorMsg() const override { return lastErrorMsg; }
    UpdateStatus testConnection(const char* token = nullptr, const char* stationId = nullptr) override;
    uint32_t getNextFetchTime() override { return nextFetchTimeMillis; }
    PriorityTier getPriorityTier() override;
    void setNextFetchTime(uint32_t forceTimeMillis) override { nextFetchTimeMillis = forceTimeMillis; }

    // Configuration & Data Access
    /**
     * @brief Configure station ID and API key.
     * @param naptanId The TfL Naptan ID (e.g. 940GZZLUBND)
     * @param apiKey Your TfL API App Key
     * @param cb Optional completion callback
     */
    void configure(const char* naptanId, const char* apiKey);
    void setFilter(const char* filter) { 
        if (filter) strlcpy(lineFilter, filter, sizeof(lineFilter)); 
        else lineFilter[0] = '\0';
    }
    void setDirectionFilter(int direction) { directionFilter = direction; }

    /**
     * @brief Limit the number of results returned. Used for lightweight testing.
     * @param limit Maximum arrivals to fetch
     */
    void setResultLimit(int limit) { maxResults = limit; }
    /**
     * @brief Enable lightweight authentication-only mode (Line Status check).
     * @param test True to enable test mode
     */
    void setTestMode(bool test) { isTestMode = test; }
    TflStation* getStationData() { return renderData.get(); }
    MessagePool* getMessagesData() { return &renderMessages; }

    // JsonListener Implementation
    void whitespace(char c) override;
    void startDocument() override;
    void key(String key) override;
    void value(String value) override;
    void endArray() override;
    void endObject() override;
    void endDocument() override;
    void startArray() override;
    void startObject() override;

public:
    /**
     * @brief Internal blocking method that executes the API protocol and coordinates JSON parse.
     */
    void executeFetch() override;
};

#endif // TFL_DATA_SOURCE_HPP
