/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/timeManager/timeManager.hpp
 * Description: Responsible for fetching network time from NTP servers
 *              and maintaining the local system clock timezone mapping.
 *
 * Exported Functions/Classes:
 * - TimeManager: [Class] Central handling for NTP synchronization.
 *   - initialize(): Performs blocking/non-blocking NTP sync.
 *   - setTimezone(): Applies POSIX timezone rules.
 *   - updateCurrentTime(): Syncs inner struct with chip RTC.
 *   - getCurrentTime(): Accesor for stabilized time struct.
 */

#pragma once

#include <Arduino.h>
#include <time.h>
#include <functional>
#include "iConfigurable.hpp"

class TimeManager : public iConfigurable {
private:
    static const char ntpServer[];
    String customTimezone;

public:
    static const char ukTimezone[];

    /**
     * @brief Define the active timezone rule explicitly.
     * @param tz Set string rule map.
     */
    void setTimezone(const String& tz);

    /**
     * @brief Fetch the current configured timezone rule string.
     * @return timezone format string
     */
    String getTimezone() const;

    /**
     * @brief Setup NTP and sync time by polling the europe.pool.ntp.org servers over WiFi.
     * @param cycleCallback Optional callback to run during polling delays.
     * @return True if sync succeeded, False if it failed after 10 attempts.
     */
    bool initialize(std::function<void()> cycleCallback = nullptr);

    /**
     * @brief Implements the iConfigurable interface.
     */
    void reapplyConfig(const Config& config) override;
    /**
     * @brief Updates the internal current time struct from the ESP32 local time.
     * @return True if time was successfully updated.
     */
    bool updateCurrentTime();

    /**
     * @brief Returns the most recently updated current time.
     * @return const struct tm reference containing local time.
     */
    const struct tm& getCurrentTime() const;

private:
    struct tm currentTime; ///< Internal tracking of the current synchronized time
};
