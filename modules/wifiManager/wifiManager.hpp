/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/wifiManager/wifiManager.hpp
 * Description: Wi-Fi credentials management and architecture-independent configuration logic.
 *
 * Exported Functions/Classes:
 * - WifiManager: Class for managing WiFi connectivity and captive portal.
 *   - begin(): Initializes WiFi and enters AP mode if no credentials found.
 *   - reapplyConfig(): Responds to hostname configuration changes.
 *   - tick(): Periodic non-blocking lifecycle management loop.
 *   - processDNS(): Intercepts captive portal DNS requests in AP mode.
 *   - updateWiFi(): Persists new WiFi credentials to LittleFS.
 *   - resetSettings(): Erases WiFi configuration and restores ESP WiFi state.
 *   - getAPMode(): Returns true if currently operating in Setup/AP Mode.
 *   - isReady(): Returns true if WiFi is ready (either STA connected or AP active).
 *   - getSSID(): Returns the current stored SSID.
 *   - hasPassword(): Returns true if a password is set.
 *   - testConnection(): Connects to standard AP momentarily to validate credentials.
 */

#ifndef WIFI_MANAGER_HPP
#define WIFI_MANAGER_HPP

#include <Arduino.h>
#include <memory>
#include <iConfigurable.hpp>

class DNSServer; // Forward declaration

/**
 * @brief Class managing WiFi connectivity and configuration.
 *        Implements iConfigurable to react to hostname changes.
 */
class WifiManager : public iConfigurable {
private:
    /**
     * @brief High-level states for the WiFi connection state machine.
     */
    enum class WiFiState {
        WIFI_INIT,             // System just started, need to decide next step
        WIFI_STA_CONNECTING,   // Trying to connect as Station
        WIFI_AP_STARTING,      // Station failed, starting AP mode
        WIFI_READY,            // Connected or AP is active
        WIFI_SHUTDOWN          // Explicitly disabled
    };

    char wifiSsid[33];
    char wifiPass[65];
    char currentHostname[33];
    bool isAPMode = false;
    WiFiState currentState = WiFiState::WIFI_INIT;
    unsigned long stateTimer = 0;
    int connectionRetries = 0;

    std::unique_ptr<DNSServer> dnsServer;

    void loadWiFiConfig();
    void saveWiFiConfig();
    void transitionTo(WiFiState newState);

public:
    WifiManager();
    ~WifiManager();

    /**
     * @brief Initialize WiFi using LittleFS credentials. If it fails, spins up an Access Point and DNS hijacker.
     * @param hostname The hostname to use for the AP and mDNS
     */
    void begin(const char* hostname);

    /**
     * @brief Implements the iConfigurable interface.
     */
    void reapplyConfig(const Config& config) override;

    /**
     * @brief Periodic maintenance tick for the WiFi state machine.
     */
    void tick();

    /**
     * @brief Handles the DNS Hijack loop when in AP mode. Should be called repeatedly in loop().
     */
    void processDNS();

    /**
     * @brief Update and save WiFi credentials.
     * @param ssid SSID to save.
     * @param pass Password to save (Masked as dots in UI).
     */
    void updateWiFi(const char* ssid, const char* pass);

    /**
     * @brief Erase stored WiFi credentials and disconnect.
     *        Clears LittleFS json and restores ESP WiFi to factory.
     */
    void resetSettings();

    /**
     * @brief Returns true if currently operating in Setup/AP Mode.
     */
    bool getAPMode() const { return isAPMode; }

    /**
     * @brief Returns true if WiFi is ready (either STA connected or AP active).
     */
    bool isReady() const { return currentState == WiFiState::WIFI_READY; }

    /**
     * @brief Returns the current stored SSID.
     */
    const char* getSSID() const { return wifiSsid; }

    /**
     * @brief Returns true if a password is set.
     */
    bool hasPassword() const { return strlen(wifiPass) > 0; }

    /**
     * @brief Test a WiFi connection without permanently changing stored credentials.
     * @param ssid SSID to test
     * @param pass Password to test (can be "****" to use current)
     * @param ipOut String to store IP on success
     * @return true on success
     */
    bool testConnection(const char* ssid, const char* pass, String& ipOut);
};

#endif
