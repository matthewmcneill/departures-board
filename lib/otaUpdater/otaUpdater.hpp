/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/otaUpdater/otaUpdater.hpp
 * Description: Encapsulates firmware lifecycle targeting GitHub releases
 *              and manages over-the-air updates.
 *
 * Exported Functions/Classes:
 * - otaUpdater: [Class] Wraps maintenance and manual update hooks.
 *   - tick: Daily maintenance log/check.
 *   - checkForFirmwareUpdate: Manual GitHub release probe and install.
 *   - checkPostWebUpgrade: Resource reconciliation after FS updates.
 * - isFirmwareUpdateAvailable: Semantic version comparison helper.
 */

#pragma once

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <githubClient.hpp>
#include "iConfigurable.hpp"
#include <LittleFS.h>
#include <functional>

/**
 * @brief Represents the current UI lifecycle state of a Firmware Update check or installation.
 */
enum class FwUpdateState {
    WARNING,
    DOWNLOADING,
    SUCCESS,
    FAILED,
    NO_UPDATES
};


extern char myUrl[24];
extern github ghUpdate;

/**
 * @brief Checks the remote GitHub repository for a firmware version greater than
 *        the current VERSION_MAJOR and VERSION_MINOR.
 * @return True if a newer release exists.
 */
bool isFirmwareUpdateAvailable();

class appContext;

class otaUpdater : public iConfigurable {
private:
    appContext* context;               // Pointer to parent context for DI
    int prevUpdateCheckDay;            // Day of month for last successful daily check
    unsigned long fwUpdateCheckTimer;  // Millis timer for debouncing interval checks
    bool updatesEnabled;               // Cached flag from config
    bool dailyCheckEnabled;            // Cached flag from config
    int quietHour;                     // Cached quiet hour from config

public:
    std::function<void(int percent)> onProgress;
    std::function<void(const char* version, int secondsRemaining)> onWarning;
    std::function<void(FwUpdateState state, const char* msg, int secondsRemaining)> onStateChange;
    std::function<void(const char* msg, int percent)> onPostUpgradeProgress;

    /**
     * @brief Initialize with the current application context.
     * @param contextPtr Pointer to the parent context.
     */
    void init(appContext* contextPtr) { context = contextPtr; }

    /**
     * @brief Access the parent application context.
     * @return appContext* Pointer to context.
     */
    appContext* getContext() const { return context; }

    void setProgressCallback(std::function<void(int percent)> cb) { onProgress = cb; }
    void setWarningCallback(std::function<void(const char* version, int secondsRemaining)> cb) { onWarning = cb; }
    void setStateCallback(std::function<void(FwUpdateState state, const char* msg, int secondsRemaining)> cb) { onStateChange = cb; }
    void setPostUpgradeCallback(std::function<void(const char* msg, int percent)> cb) { onPostUpgradeProgress = cb; }

    /**
     * @brief Default constructor.
     */
    otaUpdater();

    /**
     * @brief Main lifecycle maintenance tick. Triggers daily update checks 
     *        if enabled in configuration.
     */
    void tick();

    /**
     * @brief Forces an immediate check for newer firmware on GitHub. 
     *        Executes the update and reboots if valid firmware is found.
     * @return True if update sequence was successfully initiated.
     */
    bool checkForFirmwareUpdate();

    /**
     * @brief Checks if a firmware update is available on GitHub passively.
     * @param outVersion Populated with the remote semantic version string.
     * @return True if a newer release exists.
     */
    bool checkUpdateAvailable(String& outVersion);

    /**
     * @brief Instigates the immediate secure download loop explicitly.
     * @return True if the pipeline initiated.
     */
    bool forceUpdateNow();

    /**
     * @brief Instructs the bootloader to flip the active partition to the backup slot.
     */
    void rollbackFirmware();

    /**
     * @brief Checks for a version mismatch between the running firmware's expected
     *        Web UI and what is currently on disk. Performs cleanup if upgraded.
     */
    void checkPostWebUpgrade();

    /**
     * @brief Implements the iConfigurable interface.
     */
    void reapplyConfig(const Config& config) override;
};

extern otaUpdater ota; // Global OTA maintenance orchestrator
