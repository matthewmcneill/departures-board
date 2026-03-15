/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/webServer/webHandlerManager.cpp
 * Description: Implementation of the modern web portal handlers.
 *
 * Exported Functions/Classes:
 * - WebHandlerManager::WebHandlerManager: Constructor.
 * - WebHandlerManager::begin: Route registration.
 */

#include "webHandlerManager.hpp"
#include <webServer/portalAssets.h>
#include <ArduinoJson.h>
#include <logger.hpp>
#include <wifiManager.hpp>
#include <WiFi.h>
#include <appContext.hpp>
#include "../displayManager/boards/nationalRailBoard/nationalRailDataSource.hpp"
#include "../displayManager/boards/tflBoard/tflDataSource.hpp"

extern class appContext appContext;

WebHandlerManager::WebHandlerManager(WebServer& server, ConfigManager& config) 
    : _server(server), _config(config) {
}

void WebHandlerManager::begin() {
    LOG_INFO("WEB", "Initializing WebHandlerManager on /portal...");

    // Main Portal Entry
    _server.on("/portal", HTTP_GET, [this]() { this->handlePortalRoot(); });
    _server.on("/portal/", HTTP_GET, [this]() { this->handlePortalRoot(); });
    _server.on("/portal/index.html", HTTP_GET, [this]() { this->handlePortalRoot(); });

    // API: System & Config (Unified)
    _server.on("/api/status", HTTP_GET, [this]() { this->handleGetStatus(); });
    _server.on("/api/config", HTTP_GET, [this]() { this->handleGetConfig(); });
    _server.on("/api/saveall", HTTP_POST, [this]() { this->handleSaveAll(); });

    // API: Individual CRUD (Legacy/Granular support)
    _server.on("/api/boards", HTTP_GET, [this]() { this->handleGetBoards(); });
    _server.on("/api/boards", HTTP_POST, [this]() { this->handleSaveBoard(); });
    _server.on("/api/boards", HTTP_DELETE, [this]() { this->handleDeepDeleteBoard(); });

    // API: Keys (CRUD)
    _server.on("/api/keys", HTTP_GET, [this]() { this->handleGetKeys(); });
    _server.on("/api/keys", HTTP_POST, [this]() { this->handleSaveKey(); });
    _server.on("/api/keys", HTTP_DELETE, [this]() { this->handleDeleteKey(); });
    _server.on("/api/keys/test", HTTP_POST, [this]() { this->handleTestKey(); });

    // API: WiFi (Bespoke Captive Portal)
    _server.on("/api/wifi/scan", HTTP_GET, [this]() { this->handleWiFiScan(); });
    _server.on("/api/wifi/test", HTTP_POST, [this]() { this->handleWiFiTest(); });
    _server.on("/api/wifi/reset", HTTP_POST, [this]() { this->handleWiFiReset(); });

    // Captive Portal Redirect
    _server.onNotFound([this]() { this->handleCaptivePortalRedirect(); });

    LOG_INFO("WEB", "WebHandlerManager routes registered.");
}

void WebHandlerManager::handlePortalRoot() {
    LOG_INFO("WEB_API", "Serving /portal/index.html (gzipped)");
    sendGzipFlash(index_html_gz, index_html_gz_len, "text/html");
}

void WebHandlerManager::handleGetStatus() {
    LOG_DEBUG("WEB_API", "GET /api/status called - returning system health");
    JsonDocument doc;
    doc["heap"] = ESP.getFreeHeap();
    doc["max_alloc"] = ESP.getMaxAllocHeap();
    doc["uptime"] = millis() / 1000;
    doc["rssi"] = WiFi.RSSI();
    doc["ssid"] = WiFi.SSID();
    doc["ip"] = WiFi.localIP().toString();
    // Add Network Details
    doc["gateway"] = WiFi.gatewayIP().toString();
    doc["subnet"] = WiFi.subnetMask().toString();
    doc["dns1"] = WiFi.dnsIP(0).toString();
    doc["dns2"] = WiFi.dnsIP(1).toString();
    doc["mac"] = WiFi.macAddress();
    doc["bssid"] = WiFi.BSSIDstr();
    doc["channel"] = WiFi.channel();

    // Add LittleFS metrics
    doc["storage_total"] = LittleFS.totalBytes();
    doc["storage_used"] = LittleFS.usedBytes();

    String output;
    serializeJson(doc, output);
    _server.send(200, "application/json", output);
}

void WebHandlerManager::handleGetConfig() {
    LOG_INFO("WEB_API", "GET /api/config called - building unified JSON");
    const Config& config = _config.getConfig();
    JsonDocument doc;

    // --- System Settings ---
    JsonObject system = doc["system"].to<JsonObject>();
    system["hostname"] = config.hostname;
    system["timezone"] = config.timezone;
    system["brightness"] = config.brightness;
    system["flip"] = config.flipScreen;
    system["dateEnabled"] = config.dateEnabled;
    system["rssUrl"] = config.rssUrl;
    system["rssName"] = config.rssName;

    // --- API Key Registry ---
    JsonArray keysArr = doc["keys"].to<JsonArray>();
    for (int i = 0; i < config.keyCount; i++) {
        JsonObject kObj = keysArr.add<JsonObject>();
        kObj["id"] = config.keys[i].id;
        kObj["label"] = config.keys[i].label;
        kObj["type"] = config.keys[i].type;
        // Mask token for security
        kObj["token"] = (strlen(config.keys[i].token) > 0) ? "●●●●●●●●" : "";
    }

    // --- Boards ---
    JsonArray boardsArr = doc["boards"].to<JsonArray>();
    for (int i = 0; i < config.boardCount; i++) {
        const BoardConfig& bc = config.boards[i];
        JsonObject b = boardsArr.add<JsonObject>();
        b["type"] = (int)bc.type;
        b["id"] = bc.id;
        b["name"] = bc.name;
        b["lat"] = bc.lat;
        b["lon"] = bc.lon;
        b["filter"] = bc.filter;
        b["secId"] = bc.secondaryId;
        b["secName"] = bc.secondaryName;
        b["offset"] = bc.timeOffset;
        b["weather"] = bc.showWeather;
    }

    // --- WiFi Config (Secure) ---
    JsonObject wifi = doc["wifi"].to<JsonObject>();
    wifi["ssid"] = appContext.getWifiManager().getSSID();
    wifi["pass"] = appContext.getWifiManager().getPassMasked();

    String output;
    serializeJson(doc, output);
    _server.send(200, "application/json", output);
}

void WebHandlerManager::handleSaveAll() {
    LOG_INFO("WEB_API", "POST /api/saveall called - processing unified save");
    if (_server.hasArg("plain")) {
        LOG_DEBUG("WEB_API", String("Payload size: ") + _server.arg("plain").length() + " bytes");
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, _server.arg("plain"));
        if (error) {
            _server.send(400, "application/json", "{\"status\":\"error\",\"msg\":\"Invalid JSON\"}");
            return;
        }

        Config& config = _config.getConfig();
        
        // Update System
        if (doc["system"].is<JsonObject>()) {
            JsonObject sys = doc["system"];
            if (sys["hostname"].is<const char*>()) strlcpy(config.hostname, sys["hostname"], sizeof(config.hostname));
            if (sys["timezone"].is<const char*>()) strlcpy(config.timezone, sys["timezone"], sizeof(config.timezone));
            if (sys["brightness"].is<int>()) config.brightness = sys["brightness"];
            if (sys["flip"].is<bool>()) config.flipScreen = sys["flip"];
        }

        // Note: Keys are now managed via separate /api/keys endpoints.
        // Board apiKeyId references are updated in the boards loop below.

        // Update Boards (Full replace strategy suggested by unified save)
        if (doc["boards"].is<JsonArray>()) {
            JsonArray boardsArr = doc["boards"];
            config.boardCount = 0;
            for (JsonObject b : boardsArr) {
                if (config.boardCount >= MAX_BOARDS) break;
                BoardConfig& bc = config.boards[config.boardCount++];
                bc.type = (BoardTypes)(b["type"] | 0);
                strlcpy(bc.id, b["id"] | "", sizeof(bc.id));
                strlcpy(bc.name, b["name"] | "", sizeof(bc.name));
                bc.lat = b["lat"] | 0.0f;
                bc.lon = b["lon"] | 0.0f;
                strlcpy(bc.filter, b["filter"] | "", sizeof(bc.filter));
                strlcpy(bc.secondaryId, b["secId"] | "", sizeof(bc.secondaryId));
                strlcpy(bc.secondaryName, b["secName"] | "", sizeof(bc.secondaryName));
                bc.timeOffset = b["offset"] | 0;
                bc.showWeather = b["weather"] | true;
                strlcpy(bc.apiKeyId, b["apiKeyId"] | "", sizeof(bc.apiKeyId));
            }
        }

        // --- WiFi Credentials (Bespoke Captive Portal hand-off) ---
        if (doc["wifi"].is<JsonObject>()) {
            JsonObject w = doc["wifi"];
            const char* ssid = w["ssid"] | "";
            const char* pass = w["pass"] | "";
            if (strlen(ssid) > 0) {
                LOG_INFO("WEB_API", "Received new WiFi credentials. Saving to wifi.json...");
                appContext.getWifiManager().updateWiFi(ssid, pass);
            }
        }

        // Save System & Boards
        _config.save();

        _server.send(200, "application/json", "{\"status\":\"ok\"}");
        
        _config.notifyConsumersToReapplyConfig();

        // If we were in AP mode, we should reboot to connect to the new network
        if (appContext.getWifiManager().getAPMode()) {
            LOG_INFO("WEB_API", "System in AP mode. rebooting in 2 seconds to apply new connectivity...");
            delay(2000);
            ESP.restart();
        }
    } else {
        _server.send(400, "application/json", "{\"status\":\"error\",\"msg\":\"No data\"}");
    }
}

void WebHandlerManager::handleGetBoards() {
    // Placeholder for Phase 2 implementation
    _server.send(200, "application/json", "[]");
}

void WebHandlerManager::handleSaveBoard() {
    // Placeholder for Phase 2 implementation
    _server.send(200, "application/json", "{\"status\":\"saved\"}");
}

void WebHandlerManager::handleDeepDeleteBoard() {
    // Placeholder for Phase 2 implementation
    _server.send(200, "application/json", "{\"status\":\"deleted\"}");
}

void WebHandlerManager::handleGetKeys() {
    const Config& config = _config.getConfig();
    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();

    for (int i = 0; i < config.keyCount; i++) {
        JsonObject kObj = arr.add<JsonObject>();
        kObj["id"] = config.keys[i].id;
        kObj["label"] = config.keys[i].label;
        kObj["type"] = config.keys[i].type;
        kObj["token"] = (strlen(config.keys[i].token) > 0) ? "●●●●●●●●" : "";
    }

    String output;
    serializeJson(doc, output);
    _server.send(200, "application/json", output);
}

void WebHandlerManager::handleSaveKey() {
    if (!_server.hasArg("plain")) {
        _server.send(400, "application/json", "{\"status\":\"error\",\"msg\":\"No data\"}");
        return;
    }

    JsonDocument doc;
    deserializeJson(doc, _server.arg("plain"));
    
    ApiKey key;
    strlcpy(key.id, doc["id"] | "", sizeof(key.id));
    strlcpy(key.label, doc["label"] | "", sizeof(key.label));
    strlcpy(key.type, doc["type"] | "", sizeof(key.type));
    
    const char* token = doc["token"] | "";
    // Only update token if it's not the mask
    if (strcmp(token, "●●●●●●●●") != 0) {
        strlcpy(key.token, token, sizeof(key.token));
    } else {
        // Find existing key to preserve token
        ApiKey* existing = _config.getKeyById(key.id);
        if (existing) {
            strlcpy(key.token, existing->token, sizeof(key.token));
        }
    }

    if (strlen(key.label) == 0 || strlen(key.type) == 0) {
        _server.send(400, "application/json", "{\"status\":\"error\",\"msg\":\"Label and Type are required\"}");
        return;
    }

    _config.updateKey(key);
    _server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void WebHandlerManager::handleDeleteKey() {
    if (!_server.hasArg("id")) {
        _server.send(400, "application/json", "{\"status\":\"error\",\"msg\":\"Missing ID\"}");
        return;
    }

    _config.deleteKey(_server.arg("id").c_str());
    _server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void WebHandlerManager::handleTestKey() {
    if (!_server.hasArg("plain")) {
        _server.send(400, "application/json", "{\"status\":\"error\",\"msg\":\"No data\"}");
        return;
    }

    JsonDocument doc;
    deserializeJson(doc, _server.arg("plain"));
    const char* type = doc["type"] | "";
    const char* token = doc["token"] | "";
    
    // Resolve mask to real token if testing existing key
    if (strcmp(token, "●●●●●●●●") == 0) {
        ApiKey* existing = _config.getKeyById(doc["id"] | "");
        if (existing) token = existing->token;
    }

    bool success = false;
    String errorMsg = "Unsupported test type";

    if (strcmp(type, "owm") == 0) {
        WeatherStatus tempStatus;
        tempStatus.lat = 51.52;
        tempStatus.lon = -0.13;
        success = appContext.getWeather().updateWeather(tempStatus, nullptr, token);
        if (!success) errorMsg = appContext.getWeather().lastErrorMsg;
    } else if (strcmp(type, "rail") == 0) {
        // National Rail test - try a simple request
        nationalRailDataSource ds;
        ds.init("lite.realtime.nationalrail.co.uk", "/OpenLDBWS/wsdl.aspx?ver=2021-11-01", nullptr);
        ds.configure(token, "EUS"); // Euston
        success = (ds.updateData() == 0);
        if (!success) errorMsg = ds.getLastErrorMsg();
    } else if (strcmp(type, "tfl") == 0) {
        // TfL test
        tflDataSource ds;
        ds.configure("940GZZLUBND", token, nullptr); // Bond Street
        success = (ds.updateData() == 0);
        if (!success) errorMsg = ds.getLastErrorMsg();
    } else if (strcmp(type, "bus") == 0) {
        // bustimes.org doesn't use a token for now, but we'll return ok for the UI
        success = true;
    }

    JsonDocument res;
    res["status"] = success ? "ok" : "fail";
    if (!success) res["msg"] = errorMsg;
    
    String output;
    serializeJson(res, output);
    _server.send(200, "application/json", output);
}

void WebHandlerManager::handleWiFiScan() {
    LOG_INFO("WEB_API", "GET /api/wifi/scan - Starting WiFi scan...");
    int n = WiFi.scanNetworks();
    LOG_INFO("WEB_API", String("Scan complete. Found ") + n + " networks.");

    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();
    for (int i = 0; i < n; ++i) {
        JsonObject obj = arr.add<JsonObject>();
        obj["ssid"] = WiFi.SSID(i);
        obj["rssi"] = WiFi.RSSI(i);
        obj["secure"] = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;
    }
    
    // Clear scan results from memory
    WiFi.scanDelete();

    String output;
    serializeJson(doc, output);
    _server.send(200, "application/json", output);
}

void WebHandlerManager::handleWiFiTest() {
    LOG_INFO("WEB_API", "POST /api/wifi/test - Attempting async connection test...");
    
    if (!_server.hasArg("plain")) {
        _server.send(400, "application/json", "{\"status\":\"error\",\"msg\":\"Missing credentials\"}");
        return;
    }

    JsonDocument doc;
    deserializeJson(doc, _server.arg("plain"));
    const char* ssid = doc["ssid"] | "";
    const char* pass = doc["pass"] | "";

    if (strlen(ssid) == 0) {
        _server.send(400, "application/json", "{\"status\":\"error\",\"msg\":\"SSID required\"}");
        return;
    }

    JsonDocument res;
    String ip;
    if (appContext.getWifiManager().testConnection(ssid, pass, ip)) {
        res["status"] = "ok";
        res["ip"] = ip;
    } else {
        res["status"] = "fail";
        res["error"] = "Timeout or incorrect password";
    }

    String output;
    serializeJson(res, output);
    _server.send(200, "application/json", output);
}

void WebHandlerManager::handleWiFiReset() {
    LOG_WARN("WEB_API", "POST /api/wifi/reset called - ERASING WiFi credentials and rebooting!");
    
    // 1. Erase credentials using WiFiConfig manager
    appContext.getWifiManager().resetSettings();

    // 2. Respond to client before rebooting
    _server.send(200, "application/json", "{\"status\":\"ok\",\"msg\":\"WiFi erased. Board rebooting into AP mode...\"}");

    // 3. Trigger reboot after short delay to allow response to send
    LOG_INFO("SYSTEM", "Rebooting in 2 seconds...");
    delay(2000);
    ESP.restart();
}

void WebHandlerManager::handleCaptivePortalRedirect() {
    String host = _server.header("Host");
    LOG_INFO("WEB", String("Captive Portal: Redirecting request from ") + host + " to /portal");
    
    // Redirect all requests that are NOT for our API/Portal to the portal root
    _server.sendHeader("Location", "/portal", true);
    _server.send(302, "text/plain", "");
}

void WebHandlerManager::sendGzipFlash(const uint8_t* data, size_t len, const char* contentType) {
    _server.sendHeader("Content-Encoding", "gzip");
    _server.send_P(200, contentType, (const char*)data, len);
}
