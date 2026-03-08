#ifndef WIFI_CONFIG_HPP
#define WIFI_CONFIG_HPP

#include <Arduino.h>

extern char wifiSsid[33];
extern char wifiPass[65];

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
