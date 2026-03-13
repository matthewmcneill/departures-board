/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/WiFiConfig/WiFiConfig.hpp
 * Description: Wi-Fi credentials management and architecture-independent NVS migration logic.
 */

#ifndef WIFI_CONFIG_HPP
#define WIFI_CONFIG_HPP

#include <Arduino.h>
#include <WiFiManager.h>
#include "iConfigurable.hpp"

/**
 * @brief Class managing WiFi connectivity and configuration.
 *        Implements iConfigurable to react to hostname changes.
 */
class WiFiManagerModule : public iConfigurable {
private:
    char wifiSsid[33];
    char wifiPass[65];
    char currentHostname[33];

    void loadWiFiConfig();
    void saveWiFiConfig();

public:
    WiFiManagerModule();

    /**
     * @brief Initialize WiFi using LittleFS credentials, handling NVS migration and captive portal fallback.
     * @param hostname The hostname to use for the AP and mDNS
     * @param apCallback Optional WiFiManager callback when the portal is started
     */
    void begin(const char* hostname, void (*apCallback)(WiFiManager*));

    /**
     * @brief Implements the iConfigurable interface.
     */
    void reapplyConfig(const Config& config) override;

    /**
     * @brief Internal callback for WiFiManager saving.
     */
    void processPortalSave();
};

extern WiFiManagerModule wifiManager;

#endif
