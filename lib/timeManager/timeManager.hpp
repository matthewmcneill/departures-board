/*
 * Departures Board (c) 2025-2026 Gadec Software
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
 * Provides:
 * - TimeManager: The central class handling initialization of the system clock.
 * - timeManager: Global instantiated object.
 * - timeinfo: Standard libc `struct tm` updated globally during runs.
 */

#pragma once

#include <Arduino.h>
#include <time.h>
#include "iConfigurable.hpp"

class appContext;

class TimeManager : public iConfigurable {
private:
    appContext* context;
    static const char ntpServer[];
    String customTimezone;

public:
    static const char ukTimezone[];

    /**
     * @brief Initialize with the current application context.
     * @param contextPtr Pointer to the parent context.
     */
    void init(appContext* contextPtr) { context = contextPtr; }

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
     * @return True if sync succeeded, False if it failed after 10 attempts.
     */
    bool initialize();

    /**
     * @brief Implements the iConfigurable interface.
     */
    void reapplyConfig(const Config& config) override;
};

extern TimeManager timeManager; // Global system clock manager
extern struct tm timeinfo;      // Global synchronized time
