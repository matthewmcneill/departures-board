/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/systemManager/systemManager.hpp
 * Description: Manages global application state (boot progress, network status, 
 *              data refresh timers) extracted from Departures Board.cpp and systemBoard.cpp.
 *
 * Exported Functions/Classes:
 * - systemManager: State container and business logic orchestrator for non-display tasks.
 */

#pragma once

#include <Arduino.h>
#include <WiFi.h>

class appContext; // Forward declaration

/**
 * @brief Container for core application state and background business logic.
 */
class systemManager {
private:
    appContext* context; ///< Pointer to owner for DI access

    // --- Network State ---
    bool wifiConfigured;          ///< True if WiFiManager has valid credentials
    bool wifiConnected;           ///< True if active WiFi connection established
    unsigned long lastWiFiReconnect; ///< Last attempt timestamp (ms)
    char myUrl[24];               ///< Formatted string URL for local IP

    // --- Boot & Lifecycle State ---
    bool firstLoad;               ///< True during initial boot sequence
    int startupProgressPercent;   ///< Aggregated boot progress (0-100)
    int prevProgressBarPosition;  ///< Cached UI value to avoid redraw flicker

    // --- Data Polling State ---
    unsigned long nextDataUpdate;     ///< Calculated next poll timestamp (ms)
    unsigned long lastDataLoadTime;   ///< Timestamp of last successful data load (ms)
    bool noDataLoaded;                ///< Flag indicating if valid payload exists
    int dataLoadSuccess;              ///< Count of valid polling cycles
    int dataLoadFailure;              ///< Count of connection/parse breaks
    unsigned long lastLoadFailure;    ///< Last exception time (ms)
    int lastUpdateResult;             ///< HTTP return code from last poll

public:
    /**
     * @brief Construct the System Manager.
     */
    systemManager();

    /**
     * @brief Initialize with the current application context.
     * @param contextPtr Pointer to the parent context.
     */
    void begin(appContext* contextPtr);

    /**
     * @brief Main logic maintenance tick.
     */
    void tick();

    // --- Business Logic extracted from systemBoard.cpp ---
    void updateRssFeed();
    void addRssMessage();
    void removeRssMessage();
    void softResetBoard();
    void updateMyUrl();
    bool isAltActive();
    bool setAlternateStation();
    void tflCallback();
    void raildataCallback(int stage, int nServices);
    void updateCurrentWeather(float lat, float lon);
    String getBuildTime();

    // --- Accessors ---
    bool getWifiConnected() const { return wifiConnected; }
    void setWifiConnected(bool connected) { wifiConnected = connected; }
    
    bool getFirstLoad() const { return firstLoad; }
    void setFirstLoad(bool load) { firstLoad = load; }

    int getStartupProgressPercent() const { return startupProgressPercent; }
    void setStartupProgressPercent(int percent) { startupProgressPercent = percent; }

    unsigned long getNextDataUpdate() const { return nextDataUpdate; }
    void setNextDataUpdate(unsigned long time) { nextDataUpdate = time; }

    unsigned long getLastDataLoadTime() const { return lastDataLoadTime; }
    int getDataLoadSuccess() const { return dataLoadSuccess; }
    int getDataLoadFailure() const { return dataLoadFailure; }
    unsigned long getLastLoadFailure() const { return lastLoadFailure; }
    int getLastUpdateResult() const { return lastUpdateResult; }

    const char* getMyUrl() const { return myUrl; }

    // ... additional accessors as needed ...
};
