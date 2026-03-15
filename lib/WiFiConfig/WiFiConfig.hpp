/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/wiFiConfig/wiFiConfig.hpp
 * Description: Wi-Fi credentials management and architecture-independent NVS migration logic.
 *
 * Exported Functions/Classes:
 * - WiFiManagerModule: Singleton class for managing WiFi connectivity.
 *   - begin(): Initializes WiFi and enters AP mode if no credentials found.
 *   - updateWiFi(): Updates and persists WiFi credentials to LittleFS.
 *   - testConnection(): Validates credentials without persisting changes.
 *   - resetSettings(): Erases all WiFi configuration (NVS + LittleFS).
 *   - getAPMode(): Returns true if currently in Access Point mode.
 */

#ifndef WIFI_CONFIG_HPP
#define WIFI_CONFIG_HPP

#include <Arduino.h>
#include <Arduino.h>
#include "iConfigurable.hpp"

class DNSServer; // Forward declaration

/**
 * @brief Class managing WiFi connectivity and configuration.
 *        Implements iConfigurable to react to hostname changes.
 */
class WiFiManagerModule : public iConfigurable {
private:
    char wifiSsid[33];
    char wifiPass[65];
    char currentHostname[33];
    bool isAPMode = false;
    DNSServer* dnsServer = nullptr;

    void loadWiFiConfig();
    void saveWiFiConfig();

public:
    WiFiManagerModule();

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
     * @brief Handles the DNS Hijack loop when in AP mode. Should be called repeatedly in loop().
     */
    void processDNS();

    /**
     * @brief Update and save WiFi credentials.
     */
    void updateWiFi(const char* ssid, const char* pass);

    /**
     * @brief Erase stored WiFi credentials and disconnect.
     */
    void resetSettings();

    /**
     * @brief Returns true if currently operating in Setup/AP Mode.
     */
    bool getAPMode() const { return isAPMode; }

    /**
     * @brief Returns the current stored SSID.
     */
    const char* getSSID() const { return wifiSsid; }

    /**
     * @brief Returns a masked version of the password if set.
     */
    const char* getPassMasked() const { return (strlen(wifiPass) > 0) ? "●●●●●●●●" : ""; }

    /**
     * @brief Test a WiFi connection without permanently changing stored credentials.
     * @param ssid SSID to test
     * @param pass Password to test (can be "****" to use current)
     * @param ipOut String to store IP on success
     * @return true on success
     */
    bool testConnection(const char* ssid, const char* pass, String& ipOut);
};

extern WiFiManagerModule wifiManager;

#endif
