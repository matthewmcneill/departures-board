/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/boards/interfaces/iDataSource.hpp
 * Description: Interface for all data sources (network clients).
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

/**
 * @brief Pure virtual interface representing a source of transport data.
 *        Encapsulates the network fetching logic.
 */
class iDataSource {
public:
    virtual ~iDataSource() = default;

    /**
     * @brief Triggers the underlying mode-specific API client to fetch new data.
     * @return Status code (0 for success, non-zero for error).
     */
    virtual int updateData() = 0;

    /**
     * @brief Retrieves the last error message from the data source.
     */
    virtual const char* getLastErrorMsg() const = 0;
};

#endif // I_DATA_SOURCE_HPP
