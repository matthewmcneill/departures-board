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
 * Description: Implementation of NTP time synchronization.
 *
 * Exported Functions/Classes:
 * - TimeManager: Core class for system clock management.
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

TimeManager timeManager; // Global system clock manager singleton
struct tm timeinfo;      // Global synchronized time structure
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

bool TimeManager::initialize() {
    configTime(0, 0, ntpServer);
    setenv("TZ", ukTimezone, 1);
    tzset();
    LOG_INFO("TIME", "Time zone configured as UK default.");
    if (customTimezone != "") {
        setenv("TZ", customTimezone.c_str(), 1);
        tzset();
        LOG_INFO("TIME", String("Custom Timezone configured: ") + customTimezone);
    }

    int p = 50;
    int ntpAttempts = 0;
    bool ntpResult = true;
    LoadingBoard* load = (LoadingBoard*)context->getDisplayManager().getSystemBoard(SystemBoardId::SYS_BOOT_LOADING);
    load->setHeading("Departures Board");
    
    // Use the proxy method in systemManager to get build time
    load->setBuildTime(context->getsystemManager().getBuildTime().c_str());
    
    load->setProgress("Setting the system clock...", 50);
    context->getDisplayManager().showBoard(load);
    
    if(!getLocalTime(&timeinfo)) {
        do {
            delay(500);
            ntpResult = getLocalTime(&timeinfo);
            ntpAttempts++;
            p += 5;
            load->setProgress("Setting the system clock...", p);
            context->getDisplayManager().showBoard(load);
            if (p > 80) p = 45;
        } while ((!ntpResult) && (ntpAttempts < 10));
    }

    return ntpResult;
}
