/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/WiFiConfig/WiFiConfig.cpp
 * Description: Implementation of Wi-Fi credentials management and architecture-independent NVS migration logic.
 */
#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiConfig.hpp>
#include <Logger.hpp>

WiFiManagerModule wifiManager;

WiFiManagerModule::WiFiManagerModule() {
    memset(wifiSsid, 0, sizeof(wifiSsid));
    memset(wifiPass, 0, sizeof(wifiPass));
    memset(currentHostname, 0, sizeof(currentHostname));
}

/**
 * @brief Load Wi-Fi credentials from wifi.json on LittleFS.
 */
void WiFiManagerModule::loadWiFiConfig() {
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
      strlcpy(wifiPass, doc["pass"] | "", sizeof(wifiPass));
      
      if (strlen(wifiSsid) > 0) {
        LOG_INFO(String("Found saved WiFi credentials for SSID: ") + wifiSsid);
      }
    }
    f.close();
  }
}

/**
 * @brief Save Wi-Fi credentials to wifi.json on LittleFS.
 */
void WiFiManagerModule::saveWiFiConfig() {
  LOG_INFO("Saving WiFi credentials to wifi.json...");
  JsonDocument doc;
  doc["ssid"] = wifiSsid;
  doc["pass"] = wifiPass;

  File f = LittleFS.open("/wifi.json", "w");
  if (!f) {
    LOG_ERROR("Failed to open wifi.json for writing.");
    return;
  }
  
  serializeJson(doc, f);
  f.close();
}

/**
 * @brief Captures credentials from the portal.
 */
void wifiManagerPortalSaveCallback() {
    wifiManager.processPortalSave();
}

void WiFiManagerModule::processPortalSave() {
  LOG_INFO("WiFiManager connected. Saving entered credentials to LittleFS...");
  strlcpy(wifiSsid, WiFi.SSID().c_str(), sizeof(wifiSsid));
  strlcpy(wifiPass, WiFi.psk().c_str(), sizeof(wifiPass));
  saveWiFiConfig();
}

/**
 * @brief Core initialization routine.
 */
void WiFiManagerModule::begin(const char* hostname, void (*apCallback)(WiFiManager*)) {
  if (hostname != nullptr) strlcpy(currentHostname, hostname, sizeof(currentHostname));
  
  loadWiFiConfig();

  // Legacy Migration Check
  if (strlen(wifiSsid) == 0) {
    LOG_INFO("No wifi.json found. Checking if legacy NVS credentials exist...");
    WiFi.begin(); 
    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 15) {
      delay(500);
      retries++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      LOG_INFO("Legacy NVS WiFi connection successful! Migrating to LittleFS...");
      strlcpy(wifiSsid, WiFi.SSID().c_str(), sizeof(wifiSsid));
      strlcpy(wifiPass, WiFi.psk().c_str(), sizeof(wifiPass));
      saveWiFiConfig();
    }
  }

  // Purge buggy NVS partition (Core v3 fix)
  WiFi.disconnect(true, true);
  WiFi.persistent(false);
  
  WiFi.mode(WIFI_MODE_NULL);
  if (strlen(currentHostname) > 0) {
    WiFi.hostname(currentHostname);
  }
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(WIFI_PS_NONE);

  // Attempt connection
  if (strlen(wifiSsid) > 0) {
    LOG_INFO(String("Connecting to ") + wifiSsid + "...");
    WiFi.begin(wifiSsid, wifiPass);
    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 20) {
      delay(500);
      retries++;
    }
  }

  // Fallback to Portal
  if (WiFi.status() != WL_CONNECTED) {
    WiFiManager wm;
    wm.setSaveConfigCallback(wifiManagerPortalSaveCallback);
    if (apCallback != nullptr) wm.setAPCallback(apCallback);
    wm.setWiFiAutoReconnect(true);

    const char* portalName = (strlen(currentHostname) > 0) ? currentHostname : "Departures Board";
    if (!wm.autoConnect(portalName)) {
        LOG_ERROR("Portal timeout. Restarting.");
        ESP.restart();
    }
  }

  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
  }
}

/**
 * @brief React to configuration changes.
 */
void WiFiManagerModule::reapplyConfig(const Config& config) {
    if (strcmp(currentHostname, config.hostname) != 0) {
        LOG_INFO(String("Updating hostname to: ") + config.hostname);
        strlcpy(currentHostname, config.hostname, sizeof(currentHostname));
        WiFi.setHostname(currentHostname);
        MDNS.setInstanceName(currentHostname);
        // mDNS usually needs to be restarted or updated if the hostname changes
        MDNS.begin(currentHostname);
    }
}
