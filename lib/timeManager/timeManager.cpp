/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/timeManager/timeManager.cpp
 * Description: Implementations for the NTP clock lifecycle.
 */

#include "timeManager.hpp"
#include <U8g2lib.h>
#include <widgets/drawingPrimitives.hpp>
#include <WiFi.h>
#include <WiFiManager.h>
#include <WiFiConfig.hpp>
#include <Logger.hpp>
#include <displayManager.hpp>
#include <boards/systemBoard/systemBoard.hpp>
#include <boards/systemBoard/loadingBoard.hpp>

// --- Global Data Structures ---
TimeManager timeManager;
struct tm timeinfo;
const char* weekdays[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

// --- Constants ---
const char TimeManager::ntpServer[] = "europe.pool.ntp.org";
const char TimeManager::ukTimezone[] = "GMT0BST,M3.5.0/1,M10.5.0";

extern DisplayManager displayManager;

/**
 * @brief Configure a custom timezone string.
 * @param tz Posix TZ string.
 */
void TimeManager::reapplyConfig(const Config& config) {
    setTimezone(String(config.timezone));
}

void TimeManager::setTimezone(const String& tz) {
    customTimezone = tz;
}

/**
 * @brief Get the currently configured timezone.
 * @return String TZ identifier.
 */
String TimeManager::getTimezone() const {
    return customTimezone;
}

/**
 * @brief Initialise the system clock via NTP.
 * @return True if time was successfully syncronised.
 */
bool TimeManager::initialize() {
    configTime(0, 0, ntpServer);   // Configure NTP server for setting the clock
    setenv("TZ", ukTimezone, 1);    // Configure UK TimeZone (default and fallback if custom is invalid)
    tzset();                      // Set the TimeZone
    LOG_INFO("Time zone configured as UK default.");
    if (customTimezone != "") {
        setenv("TZ", customTimezone.c_str(), 1);
        tzset();
        LOG_INFO(String("Custom Timezone configured: ") + customTimezone);
    }

    // Check the clock has been set successfully before continuing
    int p = 50;
    int ntpAttempts = 0;
    bool ntpResult = true;
    LoadingBoard* load = (LoadingBoard*)displayManager.getSystemBoard(SystemBoardId::SYS_BOOT_LOADING);
    load->setHeading("Departures Board");
    load->setBuildTime(getBuildTime().c_str());
    load->setProgress("Setting the system clock...", 50);
    displayManager.showBoard(load);
    
    if(!getLocalTime(&timeinfo)) {              // attempt to set the clock from NTP
        do {
            delay(500);                             // If no NTP response, wait 500ms and retry
            ntpResult = getLocalTime(&timeinfo);
            ntpAttempts++;
            p += 5;
            load->setProgress("Setting the system clock...", p);
            displayManager.showBoard(load);
            if (p > 80) p = 45;
        } while ((!ntpResult) && (ntpAttempts < 10));
    }

    return ntpResult;
}
