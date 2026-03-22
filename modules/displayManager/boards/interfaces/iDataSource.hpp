/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/boards/interfaces/iDataSource.hpp
 * Description: Abstract interface for all transport data source providers. 
 *              Defines the contract for non-blocking asynchronous data fetching, 
 *              centralized task execution via DataWorker, and connectivity testing.
 *
 * Exported Functions/Classes:
 * - iDataSource: Pure virtual base class for network API clients.
 *   - updateData(): Triggers an update request (typically non-blocking).
 *   - executeFetch(): Internal blocking network operation executed by DataWorker.
 *   - getLastErrorMsg(): Human-readable error diagnostic.
 *   - testConnection(): Connectivity and credential validation logic.
 */

#ifndef I_DATA_SOURCE_HPP
#define I_DATA_SOURCE_HPP

#include <stdint.h>

// Data update status codes
#define UPD_SUCCESS 0
#define UPD_NO_CHANGE 1
#define UPD_NO_DATA 2
#define UPD_TIMEOUT 3
#define UPD_HTTP_ERROR 4
#define UPD_DATA_ERROR 5
#define UPD_UNAUTHORISED 6
#define UPD_NO_RESPONSE 7
#define UPD_INCOMPLETE 8
#define UPD_PENDING 9

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
     *        Typically enqueues this instance to the centralized DataWorker.
     * @return Status code (0 for success, non-zero for error, 9 for pending).
     */
    virtual int updateData() = 0;

    /**
     * @brief Executes the blocking network request. Must ONLY be called by 
     *        the centralized DataWorker task to prevent heap exhaustion.
     */
    virtual void executeFetch() = 0;

    /**
     * @brief Retrieves the last error message from the data source.
     */
    virtual const char* getLastErrorMsg() const = 0;

    /**
     * @brief Performs a lightweight connection and authentication test.
     * @param token Optional token to test (overrides stored configuration). Can be nullptr for data sources that do not use keys.
     * @param stationId Optional station/stop ID to test (overrides stored configuration).
     * @return 0 for success (UPD_SUCCESS), non-zero for error (UPD_*).
     */
    virtual int testConnection(const char* token = nullptr, const char* stationId = nullptr) = 0;
};

#endif // I_DATA_SOURCE_HPP
