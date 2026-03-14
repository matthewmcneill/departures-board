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
#include <WiFiConfig.hpp>
#include <WiFi.h>

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

    // API: WiFi (Bespoke Captive Portal)
    _server.on("/api/wifi/scan", HTTP_GET, [this]() { this->handleWiFiScan(); });
    _server.on("/api/wifi/test", HTTP_POST, [this]() { this->handleWiFiTest(); });

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

    // --- API Keys (Masked) ---
    JsonObject keys = doc["keys"].to<JsonObject>();
    keys["nrToken"] = (strlen(config.nrToken) > 0) ? "****" : "";
    keys["tflAppkey"] = (strlen(config.tflAppkey) > 0) ? "****" : "";
    keys["owmToken"] = (strlen(config.owmToken) > 0) ? "****" : "";

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
    wifi["ssid"] = wifiManager.getSSID();
    wifi["pass"] = wifiManager.getPassMasked();

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

        // Update Keys (Only if not masked or manually provided)
        // In this simplified version, we'll assume the client sends the real key if changed.
        if (doc["keys"].is<JsonObject>()) {
            JsonObject k = doc["keys"];
            if (k["nrToken"].is<const char*>() && strcmp(k["nrToken"], "****") != 0) strlcpy(config.nrToken, k["nrToken"], sizeof(config.nrToken));
            if (k["tflAppkey"].is<const char*>() && strcmp(k["tflAppkey"], "****") != 0) strlcpy(config.tflAppkey, k["tflAppkey"], sizeof(config.tflAppkey));
            if (k["owmToken"].is<const char*>() && strcmp(k["owmToken"], "****") != 0) strlcpy(config.owmToken, k["owmToken"], sizeof(config.owmToken));
        }

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
            }
        }

        // --- WiFi Credentials (Bespoke Captive Portal hand-off) ---
        if (doc["wifi"].is<JsonObject>()) {
            JsonObject w = doc["wifi"];
            const char* ssid = w["ssid"] | "";
            const char* pass = w["pass"] | "";
            if (strlen(ssid) > 0) {
                LOG_INFO("WEB_API", "Received new WiFi credentials. Saving to wifi.json...");
                wifiManager.updateWiFi(ssid, pass);
            }
        }

        // Save everything
        _config.save();
        
        // Key saving requires separate file in current architecture
        JsonDocument keyDoc;
        keyDoc["nrToken"] = config.nrToken;
        keyDoc["appKey"] = config.tflAppkey;
        keyDoc["owmToken"] = config.owmToken;
        String keyOutput;
        serializeJson(keyDoc, keyOutput);
        _config.saveFile("/apikeys.json", keyOutput);

        _server.send(200, "application/json", "{\"status\":\"ok\"}");
        
        _config.notifyConsumersToReapplyConfig();

        // If we were in AP mode, we should reboot to connect to the new network
        if (wifiManager.getAPMode()) {
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
    // Placeholder for Phase 2 implementation
    _server.send(200, "application/json", "[]");
}

void WebHandlerManager::handleSaveKey() {
    // Placeholder for Phase 2 implementation
    _server.send(200, "application/json", "{\"status\":\"key_saved\"}");
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
    if (wifiManager.testConnection(ssid, pass, ip)) {
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
