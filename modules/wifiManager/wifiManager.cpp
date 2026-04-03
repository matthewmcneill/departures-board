/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
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
 * - WifiManager: [Class implementation]
 *   - begin: Initial non-blocking hardware initialization.
 *   - tick: Main non-blocking state machine for connection maintenance.
 *   - updateWiFi: Configuration persistence.
 *   - resetSettings: Complete credential cleanup.
 *   - testConnection: Momentary connection probe for verification.
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
#include <deviceCrypto.hpp>


WifiManager::WifiManager() : wifiDisconnectTimer(0) {
    memset(wifiSsid, 0, sizeof(wifiSsid));
    memset(wifiPass, 0, sizeof(wifiPass));
    memset(currentHostname, 0, sizeof(currentHostname));
    strlcpy(myUrl, "http://0.0.0.0", sizeof(myUrl));
}

WifiManager::~WifiManager() {
    // std::unique_ptr automatically handles deallocation of dnsServer
}

/**
 * @brief Load Wi-Fi credentials from wifi.json on LittleFS.
 */
void WifiManager::loadWiFiConfig() {
  LOG_INFO("WIFI", "Querying local credential storage...");
  
  // --- Step 1: Detect and Migrate Legacy Configurations ---
  if (LittleFS.exists("/wifi.json")) {
    LOG_INFO("WIFI", "Legacy plaintext `/wifi.json` detected. Executing secure token migration...");
    File f = LittleFS.open("/wifi.json", "r");
    if (!f) {
      LOG_ERROR("WIFI", "Failed to open legacy setup for reading");
      return;
    }
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, f);
    if (!error) {
      strlcpy(wifiSsid, doc["ssid"] | "", sizeof(wifiSsid));
      strlcpy(wifiPass, doc["pass"] | "", sizeof(wifiPass));
    }
    f.close();
    
    // Save to encrypted bin
    if (cryptoEngine && strlen(wifiSsid) > 0) {
        saveWiFiConfig();
    }
    
    // Nuke the legacy file permanently
    LittleFS.remove("/wifi.json");
    LOG_INFO("WIFI", "Legacy credentials successfully migrated to encrypted blob and eradicated.");

  // --- Step 2: Load Secure Binary Firmware Tokens ---
  } else if (LittleFS.exists("/wifi.bin")) {
    if (cryptoEngine) {
      File f = LittleFS.open("/wifi.bin", "r");
      if (f) {
         size_t fSize = f.size();
         std::unique_ptr<char[]> fileBuf(new (std::nothrow) char[fSize + 1]);
         if (fileBuf) {
             f.readBytes(fileBuf.get(), fSize);
             fileBuf[fSize] = '\0';
             
             size_t plainLen = 0;
             std::unique_ptr<char[]> decryptedRaw = cryptoEngine->decrypt(fileBuf.get(), fSize, &plainLen);
             if (decryptedRaw && plainLen > 0) {
                 JsonDocument doc;
                 DeserializationError error = deserializeJson(doc, decryptedRaw.get());
                 if (!error) {
                    strlcpy(wifiSsid, doc["ssid"] | "", sizeof(wifiSsid));
                    strlcpy(wifiPass, doc["pass"] | "", sizeof(wifiPass));
                    if (strlen(wifiSsid) > 0) {
                      LOG_INFOf("WIFI", "Loaded secure encrypted WiFi credentials for: %s", wifiSsid);
                    }
                 } else {
                    LOG_ERROR("WIFI", "Decrypted JSON token failure. Keys misaligned?");
                 }
             }
         }
         f.close();
      }
    } else {
       LOG_ERROR("WIFI", "Security Engine unbound. Cannot decrypt standard tokens.");
    }
  }
}

/**
 * @brief Save Wi-Fi credentials to secure binary blob.
 */
void WifiManager::saveWiFiConfig() {
  LOG_INFO("WIFI", "Securing WiFi credentials...");
  if (!cryptoEngine) {
     LOG_ERROR("WIFI", "Security Engine unbound. Aborting credential save.");
     return;
  }
  
  // --- Step 1: Package Active Identity into JSON ---
  JsonDocument doc;
  doc["ssid"] = wifiSsid;
  doc["pass"] = wifiPass;
  
  size_t jsonLen = measureJson(doc);
  std::unique_ptr<char[]> plainBuf(new (std::nothrow) char[jsonLen + 1]);
  if (!plainBuf) {
      LOG_ERROR("WIFI", "Serialization memory allocation failed.");
      return;
  }
  serializeJson(doc, plainBuf.get(), jsonLen + 1);

  // --- Step 2: Encrypt Payload ---
  size_t cipherLen = 0;
  std::unique_ptr<char[]> securedBlob = cryptoEngine->encrypt(plainBuf.get(), jsonLen, &cipherLen);
  if (!securedBlob || cipherLen == 0) {
      LOG_ERROR("WIFI", "Cryptographic serialization failed.");
      return;
  }

  // --- Step 3: Write to Secure Binary File ---
  File f = LittleFS.open("/wifi.bin", "w");
  if (!f) {
    LOG_ERROR("WIFI", "Failed to construct secure binary file.");
    return;
  }
  
  f.write((uint8_t*)securedBlob.get(), cipherLen);
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
/**
 * @brief Core initialization routine (Non-Blocking).
 * Prepares the radio state but does not perform immediate connection; connectivity is managed in tick().
 * @param hostname Optional hostname to set for station/AP modes.
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
                wifiDisconnectTimer = 0; // Reset downtime tracker
                updateMyUrl();
                transitionTo(WiFiState::WIFI_READY);
            } else if (millis() - stateTimer > 15000) { // 15 second timeout for STA
                LOG_WARN("WIFI", "WiFi connection timed out. Falling back to AP.");
                WiFi.disconnect(true); // Ensure clean slate
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
                dnsServer = std::make_unique<DNSServer>();
                dnsServer->start(53, "*", apIP);
                LOG_INFO("WIFI", String("Access Point '") + portalName + "' is online.");
            }
            
            transitionTo(WiFiState::WIFI_READY);
            break;

        case WiFiState::WIFI_READY:
            // Standard operational maintenance
            processDNS();
            
            // Reconnection logic: If we're in STA mode and lost connection
            if (!isAPMode && WiFi.status() != WL_CONNECTED) {
                // If this is the first tick since disconnection, start the timer
                if (wifiDisconnectTimer == 0) {
                    wifiDisconnectTimer = millis();
                }

                // Wait 10 seconds before attempting to recycle the connection
                if (millis() - stateTimer > 10000) {
                    LOG_WARN("WIFI", "Primary connection lost. Retrying...");
                    transitionTo(WiFiState::WIFI_INIT);
                }
            } else {
                // Reset timer if we are connected or in AP mode
                stateTimer = millis();
                if (WiFi.status() == WL_CONNECTED) {
                    wifiDisconnectTimer = 0;
                }
            }
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
    // Only update if a NEW password is provided (non-empty)
    if (pass != nullptr && strlen(pass) > 0) {
        strlcpy(wifiPass, pass, sizeof(wifiPass));
    }
    saveWiFiConfig();
}

/**
 * @brief Test a WiFi connection without permanently changing stored credentials.
 * Temporarily disconnects the current session to probe a new network.
 * @param ssid SSID to test.
 * @param pass Password to test (empty string to use current stored).
 * @param ipOut [Out] String to store IP address on successful connection.
 * @return true if connection was established within timeout.
 */
bool WifiManager::testConnection(const char* ssid, const char* pass, String& ipOut) {
    LOG_INFO("WIFI", "WIFI_TEST: Starting connection test...");
    
    const char* realPass = pass;
    // If empty string provided, use the real stored password
    if (pass != nullptr && strlen(pass) == 0) {
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

    // Ensure we aren't already busy before starting a new begin
    WiFi.disconnect(true);
    delay(200); 

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
            WiFi.disconnect(true);
            delay(100);
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
    LittleFS.remove("/wifi.bin");
    
    WiFi.disconnect(true, true);
    esp_wifi_restore();
    
    LOG_INFO("WIFI", "WiFi settings purely eliminated (LittleFS + NVS restore).");
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

/**
 * @brief Update the internal URL string for the Web GUI.
 */
void WifiManager::updateMyUrl() {
    IPAddress ip = WiFi.localIP();
    snprintf(myUrl, sizeof(myUrl), "http://%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
}

/**
 * @brief Returns true if WiFi has been disconnected for longer than the timeout period (3 minutes).
 */
bool WifiManager::isWifiPersistentError() const {
    if (isAPMode || (WiFi.status() == WL_CONNECTED)) return false;
    if (wifiDisconnectTimer == 0) return false;
    
    // Over 3 minutes of continuous disconnection
    return (millis() - wifiDisconnectTimer > 180000);
}
