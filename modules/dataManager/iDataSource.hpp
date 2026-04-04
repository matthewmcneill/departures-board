/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/dataManager/iDataSource.hpp
 * Description: Abstract interface for all transport data source providers.
 *
 * Exported Functions/Classes:
 * - UpdateStatus: [Enum] Operation results.
 * - PriorityTier: [Enum] Scheduling weight.
 * - iDataSource: [Interface] pure virtual contract for API clients.
 *   - updateData(): UI-driven sync request.
 *   - executeFetch(): Worker-thread network implementation.
 *   - testConnection(): Credential validation logic.
 */

#ifndef I_DATA_SOURCE_HPP
#define I_DATA_SOURCE_HPP

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Priority tiers for data sources.
 *        Used by the centralized DataManager for predictive duty cycling.
 */
enum class PriorityTier : uint8_t {
    PRIO_CRITICAL = 0,   ///< Reserved for system-critical tests.
    PRIO_HIGH = 1,       ///< High priority (Active/Foreground).
    PRIO_MEDIUM = 2,     ///< Medium priority (Active/Foreground).
    PRIO_LOW = 3         ///< Low priority (Background/Power Saving).
};

/**
 * @brief Standardized status codes for data update operations.
 */
enum class UpdateStatus : uint8_t {
    SUCCESS = 0,      ///< Data fetch and parsing complete.
    NO_CHANGE = 1,    ///< Successfully verified no changes on server.
    NO_DATA = 2,      ///< Connection succeeded but no services returned.
    TIMEOUT = 3,      ///< Connection timed out (check network).
    HTTP_ERROR = 4,    ///< Received non-200 HTTP response.
    DATA_ERROR = 5,    ///< Connection succeeded but data format is invalid (parsing failed).
    UNAUTHORISED = 6, ///< API key rejected or expired.
    NO_RESPONSE = 7,   ///< Connected but no response received.
    INCOMPLETE = 8,    ///< Partial data received.
    PENDING = 9        ///< Data fetch is currently in progress (async).
};

/**
 * @brief Pure virtual interface representing a source of transport data.
 *        Encapsulates the network fetching logic.
 */
class iDataSource {
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~iDataSource() = default;

    /**
     * @brief Triggers the underlying mode-specific API client to fetch new data.
     *        Typically requests an update from the centralized DataManager.
     * @return UpdateStatus status code.
     */
    virtual UpdateStatus updateData() = 0;

    /**
     * @brief Executes the blocking network request. Must ONLY be called by 
     *        the centralized DataManager task to prevent heap exhaustion.
     */
    virtual void executeFetch() = 0;

    /**
     * @brief Retrieves the last error message from the data source.
     * @return Human-readable error diagnostic.
     */
    virtual const char* getLastErrorMsg() const = 0;

    /**
     * @brief Returns the next scheduled fetch time in milliseconds (millis()).
     *        Used by the centralized DataManager for predictive duty cycling.
     * @return Time in milliseconds.
     */
    virtual uint32_t getNextFetchTime() = 0;

    /**
     * @brief Returns the priority tier of this data source.
     * @return PriorityTier enum.
     */
    virtual PriorityTier getPriorityTier() = 0;

    /**
     * @brief Forces the next fetch time (in millis()).
     * @param forceTimeMillis Time in milliseconds.
     */
    virtual void setNextFetchTime(uint32_t forceTimeMillis) = 0;

    /**
     * @brief Performs a lightweight connection and authentication test.
     * @param token Optional token to test (overrides stored configuration). Can be nullptr for data sources that do not use keys.
     * @param stationId Optional station/stop ID to test (overrides stored configuration).
     * @return UpdateStatus::SUCCESS for success, otherwise an error status.
     */
    virtual UpdateStatus testConnection(const char* token = nullptr, const char* stationId = nullptr) = 0;

    /**
     * @brief Locks the data struct to prevent mid-render modification.
     */
    virtual void lockData() {}

    /**
     * @brief Unlocks the data struct.
     */
    virtual void unlockData() {}

    /**
     * @brief Gets the provider-specific semantic attribution string.
     * @return Attribution string, or nullptr if none.
     */
    virtual const char* getAttributionString() const { return nullptr; }

protected:
    uint32_t hashString(const char* str, uint32_t hash = 2166136261u) {
        if (!str) return hash;
        while (*str) {
            hash ^= (uint8_t)(*str++);
            hash *= 16777619u;
        }
        return hash;
    }
    template <typename T>
    uint32_t hashPrimitive(const T& val, uint32_t hash = 2166136261u) {
        const uint8_t* p = reinterpret_cast<const uint8_t*>(&val);
        for (size_t i = 0; i < sizeof(T); i++) {
            hash ^= p[i];
            hash *= 16777619u;
        }
        return hash;
    }
};

#endif // I_DATA_SOURCE_HPP
