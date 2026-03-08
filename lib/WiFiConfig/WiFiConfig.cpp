/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 *
 * Module: src/WiFiConfig.cpp
 * Description: Implementation of Wi-Fi credentials management and architecture-independent NVS migration logic.
 *
 * Exported Functions/Classes:
 * - setupWiFi: Initialize WiFi using LittleFS credentials, handling NVS migration and captive portal fallback.
 * - loadWiFiConfig: Load Wi-Fi credentials from wifi.json on LittleFS.
 * - saveWiFiConfig: Save Wi-Fi credentials to wifi.json on LittleFS.
 * - saveCustomWiFiCallback: Callback function to intercept saved credentials and manually store them.
 */
#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiConfig.hpp>
#include <Logger.hpp>

char wifiSsid[33] = "";
char wifiPass[65] = "";

/**
 * @brief Load Wi-Fi credentials from wifi.json on LittleFS.
 * Intended to be called on boot before attempting a standard or portal connection.
 */
void loadWiFiConfig() {
  LOG_INFO("Loading WiFi credentials from wifi.json...");
  if (LittleFS.exists("/wifi.json")) {
    File f = LittleFS.open("/wifi.json", "r");
    if (!f) {
      LOG_ERROR("Failed to open wifi.json for reading");
      return;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, f);
    if (error) {
      LOG_ERROR(String("Failed to read wifi.json: ") + error.c_str());
    } else {
      strlcpy(wifiSsid, doc["ssid"] | "", sizeof(wifiSsid));
      
      // We don't want to log the actual password
      const char* pass = doc["pass"] | "";
      strlcpy(wifiPass, pass, sizeof(wifiPass));
      
      if (strlen(wifiSsid) > 0) {
        LOG_INFO(String("Found saved WiFi credentials for SSID: ") + wifiSsid);
      } else {
        LOG_INFO("No WiFi credentials found in wifi.json");
      }
    }
    f.close();
  } else {
    LOG_INFO("wifi.json does not exist. First boot or factory reset.");
  }
}

/**
 * @brief Save Wi-Fi credentials to wifi.json on LittleFS.
 * This function persists the currently loaded wifiSsid and wifiPass character arrays.
 */
void saveWiFiConfig() {
  LOG_INFO("Saving WiFi credentials to wifi.json...");
  JsonDocument doc;
  doc["ssid"] = wifiSsid;
  doc["pass"] = wifiPass;

  File f = LittleFS.open("/wifi.json", "w");
  if (!f) {
    LOG_ERROR("Failed to open wifi.json for writing.");
    return;
  }
  
  if (serializeJson(doc, f) == 0) {
    LOG_ERROR("Failed to write to wifi.json");
  } else {
    LOG_INFO("Successfully saved WiFi credentials to wifi.json");
  }
  f.close();
}

/**
 * @brief Callback function executed by WiFiManager when the portal succeeds.
 * It grabs the plaintext credentials right before the portal exits, saving them
 * into LittleFS to maintain persistence despite NVS being disabled.
 */
void saveCustomWiFiCallback() {
  LOG_INFO("WiFiManager connected. Saving entered credentials to LittleFS...");
  
  // WiFiManager saves the captured SSID and PSK to the base WiFi library
  // We can grab them here since we've disabled persistent saving
  strlcpy(wifiSsid, WiFi.SSID().c_str(), sizeof(wifiSsid));
  strlcpy(wifiPass, WiFi.psk().c_str(), sizeof(wifiPass));
  
  saveWiFiConfig();
}

/**
 * @brief Core initialization routine orchestrating the entire Wi-Fi load/migrate/connect cycle.
 * @param hostname The hostname to advertise via mDNS and the captive portal's AP name.
 * @param apCallback An optional callback to run when WiFiManager enters AP mode (e.g. to update the display).
 */
void setupWiFi(const char* hostname, void (*apCallback)(WiFiManager*)) {
  // Attempt to load the credentials we saved manually to LittleFS
  loadWiFiConfig();

  // =========================================================================
  // BACKWARDS COMPATIBILITY MIGRATION:
  // If no wifi.json exists yet, this might be an existing board being upgraded
  // to the new LittleFS WiFi mechanism. Before we wipe the NVS, let's see
  // if the ESP already has a working WiFi connection saved in its legacy NVS.
  // =========================================================================
  if (strlen(wifiSsid) == 0) {
    LOG_INFO("No wifi.json found. Checking if legacy NVS credentials exist...");
    
    // Attempt standard connection using purely the credentials stored in the ESP's EEPROM
    WiFi.begin(); 
    
    int legacyRetries = 0;
    while (WiFi.status() != WL_CONNECTED && legacyRetries < 20) {
      delay(500);
      legacyRetries++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      LOG_INFO("Legacy NVS WiFi connection successful! Migrating to LittleFS...");
      // Extract the credentials that just successfully connected
      strlcpy(wifiSsid, WiFi.SSID().c_str(), sizeof(wifiSsid));
      strlcpy(wifiPass, WiFi.psk().c_str(), sizeof(wifiPass));
      
      // Save them permanently to the new wifi.json format
      saveWiFiConfig();
      LOG_INFO("Migration complete.");
    } else {
      LOG_INFO("No working legacy NVS credentials found.");
    }
  }

  // =========================================================================
  // FIX: WiFi Credentials NVS Bug in ESP32 Arduino Core v3.x
  // =========================================================================
  // The ESP32 Arduino Core v3.x has a known bug where writing to the default
  // WiFi Non-Volatile Storage (NVS) partition during WiFiManager captive portal
  // execution can trigger a watchdog timeout or "core v3 panic", leading to a
  // boot loop. 
  // 
  // To bypass this, we MUST:
  // 1. Force wipe the buggy credentials from the NVS: WiFi.disconnect(true, true);
  // 2. Disable NVS saving completely: WiFi.persistent(false);
  // 3. Keep our own copy in LittleFS (wifi.json) using the WiFiConfig routines!
  // =========================================================================
  WiFi.disconnect(true, true);      // PURGE corrupted WiFi NVS configuration from previous crashes
  WiFi.persistent(false);           // Disable NVS WiFi saving to prevent core v3 panics
  
  WiFi.mode(WIFI_MODE_NULL);        // Reset the WiFi
  if (hostname != nullptr) {
    WiFi.hostname(hostname);        // Set the hostname ("Departures Board")
  }
  WiFi.mode(WIFI_STA);              // Enter WiFi station mode
  WiFi.setSleep(WIFI_PS_NONE);      // Turn off WiFi Powersaving

  // If we have an SSID on file, attempt to connect directly outside the portal first
  if (strlen(wifiSsid) > 0) {
    LOG_INFO(String("Attempting to connect to saved WiFi network: ") + wifiSsid);
    WiFi.begin(wifiSsid, wifiPass);
    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 20) {
      delay(500);
      retries++;
    }
    if (WiFi.status() == WL_CONNECTED) {
      LOG_INFO("Successfully connected to saved WiFi network.");
    } else {
      LOG_WARN("Failed to connect to saved WiFi network. Falling back to portal.");
    }
  }

  // If we couldn't connect manually, spin up the WiFiManager captive portal
  if (WiFi.status() != WL_CONNECTED) {
    WiFiManager wm;                   // Start WiFiManager
    
    // Attach our custom callback so we can grab the SSID/Pass the user enters
    // and save it to LittleFS before the portal shuts down.
    wm.setSaveConfigCallback(saveCustomWiFiCallback);

    if (apCallback != nullptr) {
      wm.setAPCallback(apCallback);     // Set the callback for config mode notification
    }
    wm.setWiFiAutoReconnect(true);              // Attempt to auto-reconnect WiFi
    wm.setConnectTimeout(8);
    wm.setConnectRetries(2);

    LOG_INFO("Starting WiFiManager AutoConnect...");
    delay(1000); // Flush logs
    
    const char* portalName = (hostname != nullptr) ? hostname : "Departures Board";
    bool result = wm.autoConnect(portalName);    // Attempt to connect to WiFi (or enter interactive configuration mode)
    
    LOG_INFO("WiFiManager AutoConnect returned.");
    delay(1000); // Flush logs
    if (!result) {
        LOG_ERROR("WiFiManager AutoConnect failed. Restarting device.");
        // Failed to connect/configure
        ESP.restart();
    }
  }

  // Wait for WiFi connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
  }
}
