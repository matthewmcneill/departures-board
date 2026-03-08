/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 *
 * Module: include/WiFiConfig.hpp
 * Description: Wi-Fi credentials management and architecture-independent NVS migration logic.
 *
 * Exported Functions/Classes:
 * - setupWiFi: Initialize WiFi using LittleFS credentials, handling NVS migration and captive portal fallback.
 * - loadWiFiConfig: Load Wi-Fi credentials from wifi.json on LittleFS.
 * - saveWiFiConfig: Save Wi-Fi credentials to wifi.json on LittleFS.
 * - saveCustomWiFiCallback: Callback function to intercept saved credentials and manually store them.
 */
#ifndef WIFI_CONFIG_HPP
#define WIFI_CONFIG_HPP

#include <Arduino.h>
#include <WiFiManager.h>

extern char wifiSsid[33];
extern char wifiPass[65];

/**
 * @brief Initialize WiFi using LittleFS credentials, handling NVS migration and captive portal fallback.
 * @param hostname The hostname to use for the AP and mDNS
 * @param apCallback Optional WiFiManager callback when the portal is started
 */
void setupWiFi(const char* hostname, void (*apCallback)(WiFiManager*));

/**
 * @brief Load Wi-Fi credentials from wifi.json on LittleFS.
 */
void loadWiFiConfig();

/**
 * @brief Save Wi-Fi credentials to wifi.json on LittleFS.
 */
void saveWiFiConfig();

/**
 * @brief Callback function to be passed to WiFiManager to intercept saved credentials and manually store them.
 */
void saveCustomWiFiCallback();

#endif
