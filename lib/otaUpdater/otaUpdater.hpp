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
 *              and manages over-the-air updates from the Web UI.
 *
 * Exported Functions/Classes:
 * - isFirmwareUpdateAvailable: Logic to compare local version with remote tags.
 * - otaUpdater: Class wrapping daily maintenance and manual update hooks.
 * - ota: Global orchestration instance.
 */

#pragma once

#include <Arduino.h>
#include <widgets/drawingPrimitives.hpp>
#include <WiFiClientSecure.h>
#include <githubClient.hpp>
#include "iConfigurable.hpp"
#include <LittleFS.h>
#include <boards/systemBoard/loadingBoard.hpp>

extern int VERSION_MAJOR;
extern int VERSION_MINOR;
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

public:
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
