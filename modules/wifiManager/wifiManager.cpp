/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/wifiManager/wifiManager.cpp
 * Description: Implementation of Wi-Fi credentials management and architecture-independent configuration logic.
 *
 * Exported Functions/Classes:
 * - WifiManager: Class for managing WiFi connectivity and captive portal.
 *   - begin(): Initializes WiFi and enters AP mode if no credentials found.
 *   - reapplyConfig(): Responds to hostname configuration changes.
 *   - tick(): Periodic non-blocking lifecycle management loop.
 *   - processDNS(): Intercepts captive portal DNS requests in AP mode.
 *   - updateWiFi(): Persists new WiFi credentials to LittleFS.
 *   - resetSettings(): Erases WiFi configuration and restores ESP WiFi state.
 *   - testConnection(): Connects to standard AP momentarily to validate credentials.
 */
#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <wifiManager.hpp>
#include <logger.hpp>
#include <esp_wifi.h>


WifiManager::WifiManager() {
    memset(wifiSsid, 0, sizeof(wifiSsid));
    memset(wifiPass, 0, sizeof(wifiPass));
    memset(currentHostname, 0, sizeof(currentHostname));
}

/**
 * @brief Load Wi-Fi credentials from wifi.json on LittleFS.
 */
void WifiManager::loadWiFiConfig() {
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
void WifiManager::saveWiFiConfig() {
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
 * @brief transitionTo helper to log and reset timers.
 */
void WifiManager::transitionTo(WiFiState newState) {
    currentState = newState;
    stateTimer = millis();
}

/**
 * @brief Core initialization routine (Non-Blocking).
 */
void WifiManager::begin(const char* hostname) {
    if (hostname != nullptr) strlcpy(currentHostname, hostname, sizeof(currentHostname));
    loadWiFiConfig();

    // Prepare for tick-based init
    WiFi.persistent(false);
    WiFi.disconnect(true, true);
    WiFi.mode(WIFI_OFF);
    
    transitionTo(WiFiState::WIFI_INIT);
    LOG_INFO("WIFI", "WiFi Manager initialized in NON-BLOCKING mode.");
}

/**
 * @brief Periodic maintenance tick for the WiFi state machine.
 */
void WifiManager::tick() {
    switch (currentState) {
        case WiFiState::WIFI_INIT:
            // Wait 2 seconds for system stability before touching the radio
            if (millis() - stateTimer > 2000) {
                if (strlen(wifiSsid) > 0) {
                    LOG_INFO("WIFI", String("Connecting to ") + wifiSsid + "...");
                    WiFi.mode(WIFI_STA);
                    WiFi.setSleep(WIFI_PS_NONE);
                    if (strlen(currentHostname) > 0) WiFi.setHostname(currentHostname);
                    WiFi.begin(wifiSsid, wifiPass);
                    connectionRetries = 0;
                    transitionTo(WiFiState::WIFI_STA_CONNECTING);
                } else {
                    LOG_WARN("WIFI", "No credentials found. Falling back to AP mode.");
                    transitionTo(WiFiState::WIFI_AP_STARTING);
                }
            }
            break;

        case WiFiState::WIFI_STA_CONNECTING:
            if (WiFi.status() == WL_CONNECTED) {
                LOG_INFO("WIFI", String("Connected successfully. IP: ") + WiFi.localIP().toString());
                isAPMode = false;
                transitionTo(WiFiState::WIFI_READY);
            } else if (millis() - stateTimer > 15000) { // 15 second timeout for STA
                LOG_WARN("WIFI", "WiFi connection timed out. Falling back to AP.");
                WiFi.disconnect();
                transitionTo(WiFiState::WIFI_AP_STARTING);
            }
            break;

        case WiFiState::WIFI_AP_STARTING:
            LOG_INFO("WIFI", "Starting Access Point setup (Captive Portal)...");
            isAPMode = true;
            WiFi.mode(WIFI_AP);
            
            {
                IPAddress apIP(192, 168, 4, 1);
                WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
                
                const char* portalName = (strlen(currentHostname) > 0) ? currentHostname : "Departures-Board";
                WiFi.softAP(portalName);
                
                // Start DNS Hijacker
                if (dnsServer) delete dnsServer;
                dnsServer = new DNSServer();
                dnsServer->start(53, "*", apIP);
                LOG_INFO("WIFI", String("Access Point '") + portalName + "' is online.");
            }
            
            transitionTo(WiFiState::WIFI_READY);
            break;

        case WiFiState::WIFI_READY:
            // Standard operational maintenance
            processDNS();
            break;

        case WiFiState::WIFI_SHUTDOWN:
            break;
    }
}

/**
 * @brief Handles DNS requests when in AP mode.
 */
void WifiManager::processDNS() {
    if (isAPMode && dnsServer != nullptr) {
        dnsServer->processNextRequest();
    }
}

/**
 * @brief Update and save WiFi credentials.
 */
void WifiManager::updateWiFi(const char* ssid, const char* pass) {
    if (ssid != nullptr) strlcpy(wifiSsid, ssid, sizeof(wifiSsid));
    // Skip if password is the UI mask "●●●●●●●●"
    if (pass != nullptr && strcmp(pass, "●●●●●●●●") != 0) {
        strlcpy(wifiPass, pass, sizeof(wifiPass));
    }
    saveWiFiConfig();
}

bool WifiManager::testConnection(const char* ssid, const char* pass, String& ipOut) {
    LOG_INFO("WIFI", "WIFI_TEST: Starting connection test...");
    
    const char* realPass = pass;
    // If masked, use the real stored password
    if (strcmp(pass, "●●●●●●●●") == 0) {
        LOG_INFO("WIFI", "WIFI_TEST: Using stored credentials for test.");
        realPass = wifiPass;
    }

    // Optimization: If already connected to this IDENTICAL SSID/PASS, skip re-connecting
    // This prevents dropping the current web session during the test.
    if (WiFi.status() == WL_CONNECTED && strcmp(WiFi.SSID().c_str(), ssid) == 0 && strcmp(realPass, wifiPass) == 0) {
        ipOut = WiFi.localIP().toString();
        LOG_INFO("WIFI", "WIFI_TEST: Already connected to this network. IP: " + ipOut);
        return true;
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
void WifiManager::resetSettings() {
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
void WifiManager::reapplyConfig(const Config& config) {
    if (strcmp(currentHostname, config.hostname) != 0) {
        LOG_INFO("WIFI", String("Updating hostname to: ") + config.hostname);
        strlcpy(currentHostname, config.hostname, sizeof(currentHostname));
        WiFi.setHostname(currentHostname);
        MDNS.setInstanceName(currentHostname);
        // mDNS usually needs to be restarted or updated if the hostname changes
        MDNS.begin(currentHostname);
    }
}
