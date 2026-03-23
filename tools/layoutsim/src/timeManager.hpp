/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: tools/layoutsim/src/timeManager.hpp
 * Description: Retrieves host-bound local time variables for WebAssembly mock widgets.
 *
 * Exported Functions/Classes:
 * - TimeManager: Time retrieval interface for clock widgets
 *   - updateCurrentTime(): Re-fetches time structure from C standard library
 *   - getCurrentTime(): Returns a modifiable struct tm reference
 */

#ifndef MOCK_TIME_MANAGER_HPP
#define MOCK_TIME_MANAGER_HPP

#include <time.h>
#include <stdint.h>

class TimeManager {
private:
    struct tm currentTime;

public:
    TimeManager() {
        updateCurrentTime();
    }

    /**
     * @brief Re-fetches time structure from C standard library
     */
    void updateCurrentTime() {
        time_t now = time(NULL);
        struct tm *local = localtime(&now);
        if (local) {
            currentTime = *local;
        }
    }

    /**
     * @brief Returns a modifiable struct tm reference
     * @return Reference to internal C stdlib time container
     */
    struct tm& getCurrentTime() {
        return currentTime;
    }
};

#endif // MOCK_TIME_MANAGER_HPP
