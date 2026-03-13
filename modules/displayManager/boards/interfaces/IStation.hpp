/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/boards/BoardInterfaces/IStation.hpp
 * Description: Pure virtual interface representing a Station or Stop (e.g. paddington, or a bus stop).
 *              It encapsulates the underlying data fetching implementation and 
 *              mandates how the board should visually render itself on the OLED.
 *
 * Exported Assets/Functions:
 * - IStation: Core interface class for defining station behaviour and OLED rendering.
 */

#ifndef I_STATION_HPP
#define I_STATION_HPP

#include <U8g2lib.h>
#include <U8g2lib.h>

#define UPD_SUCCESS 0      // Data update completed successfully without errors
#define UPD_INCOMPLETE 1   // Data update finished but response was incomplete or truncated
#define UPD_UNAUTHORISED 2 // Data update failed due to invalid API credentials
#define UPD_HTTP_ERROR 3   // Data update failed due to a non-200 HTTP response code
#define UPD_TIMEOUT 4      // Data update aborted because the API response took too long
#define UPD_NO_RESPONSE 5  // Data update aborted because the server never responded
#define UPD_DATA_ERROR 6   // Data update failed because the returned payload was malformed
#define UPD_NO_CHANGE 7    // Data update skipped because the remote payload hasn't changed

class IStation {
public:
    virtual ~IStation() = default;

    /**
     * @return The human-readable name of the location being displayed.
     */
    virtual const char* getLocationName() const = 0;

    // --- API Data Fetching ---

    /**
     * @brief Triggers the underlying mode-specific API client to fetch new data.
     * @return Connection status constant (e.g. UPD_SUCCESS).
     */
    virtual int updateData() = 0;

    /**
     * @brief Retrieves the last error message from the API client.
     */
    virtual const char* getLastErrorMsg() const = 0;

    // --- OLED Rendering Hooks ---
    
    /**
     * @brief Render the entire board layout to the display buffer based on internal state.
     * @param display Reference to the global U8g2 object.
     */
    virtual void render(U8G2& display) = 0;

    /**
     * @brief Called periodically inside the main loop to manage animation and timing.
     */
    virtual void tick(uint32_t currentMillis) = 0;

    // --- Debug ---

    /**
     * @brief Dumps the current station and service data to the Serial monitor.
     */
    virtual void dumpToSerial() const = 0;
};

#endif // I_STATION_HPP
