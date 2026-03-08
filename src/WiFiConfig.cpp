#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include "WiFiConfig.hpp"
#include "Logger.hpp"

char wifiSsid[33] = "";
char wifiPass[65] = "";

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

void saveCustomWiFiCallback() {
  LOG_INFO("WiFiManager connected. Saving entered credentials to LittleFS...");
  
  // WiFiManager saves the captured SSID and PSK to the base WiFi library
  // We can grab them here since we've disabled persistent saving
  strlcpy(wifiSsid, WiFi.SSID().c_str(), sizeof(wifiSsid));
  strlcpy(wifiPass, WiFi.psk().c_str(), sizeof(wifiPass));
  
  saveWiFiConfig();
}
