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
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <WiFiConfig.hpp>
#include <Logger.hpp>
#include <esp_wifi.h>

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
  LOG_INFO("WIFI", "Loading WiFi credentials from wifi.json...");
  if (LittleFS.exists("/wifi.json")) {
    File f = LittleFS.open("/wifi.json", "r");
    if (!f) {
      LOG_ERROR("WIFI", "Failed to open wifi.json for reading");
      return;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, f);
    if (error) {
      LOG_ERROR("WIFI", String("Failed to read wifi.json: ") + error.c_str());
    } else {
      strlcpy(wifiSsid, doc["ssid"] | "", sizeof(wifiSsid));
      strlcpy(wifiPass, doc["pass"] | "", sizeof(wifiPass));
      
      if (strlen(wifiSsid) > 0) {
        LOG_INFO("WIFI", String("Found saved WiFi credentials for SSID: ") + wifiSsid);
      }
    }
    f.close();
  }
}

/**
 * @brief Save Wi-Fi credentials to wifi.json on LittleFS.
 */
void WiFiManagerModule::saveWiFiConfig() {
  LOG_INFO("WIFI", "Saving WiFi credentials to wifi.json...");
  JsonDocument doc;
  doc["ssid"] = wifiSsid;
  doc["pass"] = wifiPass;

  File f = LittleFS.open("/wifi.json", "w");
  if (!f) {
    LOG_ERROR("WIFI", "Failed to open wifi.json for writing.");
    return;
  }
  
  serializeJson(doc, f);
  f.close();
}

/**
 * @brief Core initialization routine.
 */
void WiFiManagerModule::begin(const char* hostname) {
  if (hostname != nullptr) strlcpy(currentHostname, hostname, sizeof(currentHostname));
  
  loadWiFiConfig();

  // Legacy Migration Check
  if (strlen(wifiSsid) == 0) {
    LOG_INFO("WIFI", "No wifi.json found. Checking if legacy NVS credentials exist...");
    WiFi.begin(); 
    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 15) {
      delay(500);
      retries++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      LOG_INFO("WIFI", "Legacy NVS WiFi connection successful! Migrating to LittleFS...");
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
    LOG_INFO("WIFI", String("Connecting to ") + wifiSsid + "...");
    WiFi.begin(wifiSsid, wifiPass);
    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 20) {
      delay(500);
      retries++;
    }
  }

  // Fallback to Bespoke Captive Portal
  if (WiFi.status() != WL_CONNECTED) {
    LOG_INFO("WIFI", "Connection failed or no credentials. Starting AP Mode (Captive Portal).");
    isAPMode = true;
    
    // Switch to Access Point mode
    WiFi.mode(WIFI_AP);
    
    // Set AP IP manually (standard captive portal IP)
    IPAddress apIP(192, 168, 4, 1);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    
    const char* portalName = (strlen(currentHostname) > 0) ? currentHostname : "Departures-Board";
    WiFi.softAP(portalName);
    
    LOG_INFO("WIFI", String("Access Point started: ") + portalName);
    LOG_INFO("WIFI", String("AP IP address: ") + WiFi.softAPIP().toString());

    LOG_INFO("WIFI", String("AP IP address: ") + WiFi.softAPIP().toString());

    // Start DNS Hijacker
    if (dnsServer != nullptr) delete dnsServer;
    dnsServer = new DNSServer();
    dnsServer->start(53, "*", apIP);
    LOG_INFO("WIFI", "DNS Server started on port 53 (Hijacking all requests to AP).");
  } else {
    isAPMode = false;
    LOG_INFO("WIFI", String("Connected successfully. IP: ") + WiFi.localIP().toString());
  }
}

/**
 * @brief Handles DNS requests when in AP mode.
 */
void WiFiManagerModule::processDNS() {
    if (isAPMode && dnsServer != nullptr) {
        dnsServer->processNextRequest();
    }
}

/**
 * @brief Update and save WiFi credentials.
 */
void WiFiManagerModule::updateWiFi(const char* ssid, const char* pass) {
    if (ssid != nullptr) strlcpy(wifiSsid, ssid, sizeof(wifiSsid));
    // Skip if password is the UI mask "****"
    if (pass != nullptr && strcmp(pass, "****") != 0) {
        strlcpy(wifiPass, pass, sizeof(wifiPass));
    }
    saveWiFiConfig();
}

bool WiFiManagerModule::testConnection(const char* ssid, const char* pass, String& ipOut) {
    LOG_INFO("WIFI", "WIFI_TEST: Starting connection test...");
    
    const char* realPass = pass;
    // If masked, use the real stored password
    if (strcmp(pass, "****") == 0) {
        LOG_INFO("WIFI", "WIFI_TEST: Using stored credentials for test.");
        realPass = wifiPass;
    }

    WiFi.begin(ssid, realPass);
    
    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 20) {
        delay(500);
        retries++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        ipOut = WiFi.localIP().toString();
        LOG_INFO("WIFI", "WIFI_TEST: Success! IP: " + ipOut);
        return true;
    } else {
        LOG_WARN("WIFI", "WIFI_TEST: Failed. Reconnecting to primary credentials.");
        // Revert to known-good credentials immediately
        if (strlen(wifiSsid) > 0) {
            WiFi.begin(wifiSsid, wifiPass);
        }
        return false;
    }
}

/**
 * @brief Erase stored WiFi credentials and disconnect.
 */
void WiFiManagerModule::resetSettings() {
    memset(wifiSsid, 0, sizeof(wifiSsid));
    memset(wifiPass, 0, sizeof(wifiPass));
    LittleFS.remove("/wifi.json");
    
    WiFi.disconnect(true, true);
    esp_wifi_restore();
    
    LOG_INFO("WIFI", "WiFi settings thoroughly erased (LittleFS + NVS restore).");
}

/**
 * @brief React to configuration changes.
 */
void WiFiManagerModule::reapplyConfig(const Config& config) {
    if (strcmp(currentHostname, config.hostname) != 0) {
        LOG_INFO("WIFI", String("Updating hostname to: ") + config.hostname);
        strlcpy(currentHostname, config.hostname, sizeof(currentHostname));
        WiFi.setHostname(currentHostname);
        MDNS.setInstanceName(currentHostname);
        // mDNS usually needs to be restarted or updated if the hostname changes
        MDNS.begin(currentHostname);
    }
}
