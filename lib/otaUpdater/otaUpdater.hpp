/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/otaUpdater/otaUpdater.hpp
 * Description: Encapsulates firmware lifecycle targeting GitHub releases
 *              and manages over-the-air updates from the Web UI.
 *
 * Provides:
 * - OtaUpdater: Class wrapping the update cycles, daily interval checks, and network requests.
 * - ota: Global orchestration instance.
 * - isFirmwareUpdateAvailable(): Checks runtime build version against GitHub tags.
 */

#pragma once

#include <Arduino.h>
#include <drawingPrimitives.hpp>
#include "../boards/systemBoard/include/systemBoard.hpp"
#include <WiFiClientSecure.h>
#include <githubClient.h>

extern int VERSION_MAJOR;
extern int VERSION_MINOR;
extern github ghUpdate;

/**
 * @brief Check GitHub if a newer firmware than our VERSION_MAJOR and VERSION_MINOR exists
 */
bool isFirmwareUpdateAvailable();

class OtaUpdater {
private:
    bool firmwareUpdatesEnabled;
    bool dailyUpdateCheckEnabled;
    int prevUpdateCheckDay;
    unsigned long fwUpdateCheckTimer;

public:
    OtaUpdater();

    /**
     * @brief Configure OTA update settings
     */
    void configure(bool firmwareUpdates, bool dailyUpdateCheck);

    bool getFirmwareUpdatesEnabled() const;
    void setFirmwareUpdatesEnabled(bool enabled);

    bool getDailyUpdateCheckEnabled() const;
    void setDailyUpdateCheckEnabled(bool enabled);

    /**
     * @brief Main lifecycle tick to check for daily scheduled updates
     */
    void tick();

    /**
     * @brief Attempts to install newer firmware if available and reboots
     */
    bool checkForFirmwareUpdate();
};

extern OtaUpdater ota;
