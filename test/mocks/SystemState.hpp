/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: test/mocks/SystemState.hpp
 * Description: Singleton mock for injecting global system states (WiFi, Weather, OTA).
 */

#ifndef SYSTEM_STATE_MOCK_HPP
#define SYSTEM_STATE_MOCK_HPP

#include <stdint.h>

/**
 * @class SystemState
 * @brief Provides a central point for logic injection in tests and simulation.
 */
class SystemState {
public:
    static SystemState& getInstance() {
        static SystemState instance;
        return instance;
    }

    // --- WiFi Status ---
    bool isWifiConnected() const { return wifiConnected; }
    void setWifiConnected(bool connected) { wifiConnected = connected; }

    // --- Weather Status ---
    bool isWeatherAvailable() const { return weatherAvailable; }
    void setWeatherAvailable(bool available) { weatherAvailable = available; }
    int getTemperature() const { return temperature; }
    void setTemperature(int temp) { temperature = temp; }

    // --- OTA Status ---
    bool isOtaInProgress() const { return otaInProgress; }
    void setOtaInProgress(bool inProgress) { otaInProgress = inProgress; }
    int getOtaProgress() const { return otaProgress; }
    void setOtaProgress(int progress) { otaProgress = progress; }

    // --- Time Sync ---
    bool isTimeSynced() const { return timeSynced; }
    void setTimeSynced(bool synced) { timeSynced = synced; }

    /** @brief Reset all states to physical "Happy Path" defaults. */
    void reset() {
        wifiConnected = true;
        weatherAvailable = true;
        temperature = 22;
        otaInProgress = false;
        otaProgress = 0;
        timeSynced = true;
    }

private:
    SystemState() { reset(); }
    
    bool wifiConnected;
    bool weatherAvailable;
    int temperature;
    bool otaInProgress;
    int otaProgress;
    bool timeSynced;
};

#endif // SYSTEM_STATE_MOCK_HPP
