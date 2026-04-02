/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/timeManager/timeManager.cpp
 * Description: Implementation of NTP time synchronization and timezone handling.
 *
 * Exported Functions/Classes:
 * - TimeManager: [Class implementation]
 *   - initialize: Boot-time NTP sync loop with status callback.
 *   - setTimezone: POSIX rule injection.
 *   - updateCurrentTime: Local RTC polling.
 */

#include "timeManager.hpp"
#include <U8g2lib.h>
#include <widgets/drawingPrimitives.hpp>
#include <WiFi.h>
#include <wifiManager.hpp>
#include <logger.hpp>
#include <displayManager.hpp>
#include <boards/systemBoard/loadingBoard.hpp>
#include <WebServer.h>
#include <appContext.hpp>
#include <functional>

// Globals removed in favor of injected TimeManager instance
const char* weekdays[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"}; // Weekday labels

const char TimeManager::ntpServer[] = "europe.pool.ntp.org";
const char TimeManager::ukTimezone[] = "GMT0BST,M3.5.0/1,M10.5.0";

void TimeManager::reapplyConfig(const Config& config) {
    setTimezone(String(config.timezone));
}

void TimeManager::setTimezone(const String& tz) {
    customTimezone = tz;
}

String TimeManager::getTimezone() const {
    return customTimezone;
}

bool TimeManager::updateCurrentTime() {
    return getLocalTime(&currentTime);
}

const struct tm& TimeManager::getCurrentTime() const {
    return currentTime;
}


/**
 * @brief Initialize the system clock via NTP.
 * This is a potentially blocking operation that polls the NTP pool.
 * @param cycleCallback Optional function to run between poll attempts (e.g. for display updates).
 * @return true if time was successfully synchronized.
 */
bool TimeManager::initialize(std::function<void()> cycleCallback) {
    // --- Step 1: Base Configuration ---
    // Register the standard NTP pool and applying the fallback timezone
    configTime(0, 0, ntpServer);
    setenv("TZ", ukTimezone, 1);
    tzset();
    LOG_INFO("TIME", "Time zone configured as UK default.");
    
    // Override with custom timezone if provided by config
    if (customTimezone != "") {
        setenv("TZ", customTimezone.c_str(), 1);
        tzset();
        LOG_INFO("TIME", String("Custom Timezone configured: ") + customTimezone);
    }

    // --- Step 2: NTP Synchronization ---
    // Poll the network until a valid time is received or we exhaust our 10 retries
    int ntpAttempts = 0;
    bool ntpResult = true;
    
    if(!getLocalTime(&currentTime)) {
        do {
            delay(500);
            ntpResult = getLocalTime(&currentTime);
            ntpAttempts++;
            if (cycleCallback) {
                cycleCallback();
            }
        } while ((!ntpResult) && (ntpAttempts < 10));
    }

    return ntpResult;
}
