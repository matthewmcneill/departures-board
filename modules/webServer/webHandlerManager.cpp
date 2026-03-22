/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
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
 * - WebHandlerManager::handlePortalRoot: Serves the main web index.html.
 * - WebHandlerManager::handleGetStatus: Returns system health, connectivity metrics, time and timezone.
 * - WebHandlerManager::handleGetConfig: Returns the unified project configuration as JSON.
 * - WebHandlerManager::handleSaveAll: Atomic validation and save of all portal settings.
 * - WebHandlerManager::handleReboot: Restarts the ESP32 device.
 * - WebHandlerManager::handleOTACheck: Triggers a manual firmware update check.
 * - WebHandlerManager::handleRSSJson: Serves the gzipped rss.json asset.
 */

#include "webHandlerManager.hpp"
// Web UI Assets Embed
#include "portalAssets.h"
#include <ArduinoJson.h>
#include <logger.hpp>
#include <wifiManager.hpp>
#include <WiFi.h>
#include <appContext.hpp>
#include "../displayManager/boards/nationalRailBoard/nationalRailDataSource.hpp"
#include "../displayManager/boards/tflBoard/tflDataSource.hpp"
#include "../displayManager/boards/busBoard/busDataSource.hpp"
#include "../../lib/rssClient/rssClient.hpp"
#include "../weatherClient/weatherClient.hpp"

extern class appContext appContext;

WebHandlerManager::WebHandlerManager(AsyncWebServer& server, ConfigManager& config) 
    : _server(server), _config(config) {
}

void WebHandlerManager::begin() {
    LOG_INFO("WEB", "Initializing WebHandlerManager on /portal...");

    // Helper lambda to collect chunked POST bodies and pass to handlers
    auto bindPostDynamic = [this](const char* uri, void (WebHandlerManager::*handlerFunc)(AsyncWebServerRequest*, const String&)) {
        _server.on(uri, HTTP_POST, 
            [this, handlerFunc](AsyncWebServerRequest *request) {
                if (request->_tempObject) {
                    String* bodyPtr = (String*)request->_tempObject;
                    String body = *bodyPtr;
                    delete bodyPtr;
                    request->_tempObject = NULL;
                    (this->*handlerFunc)(request, body);
                } else {
                    (this->*handlerFunc)(request, "");
                }
            },
            NULL,
            [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
                if (index == 0) {
                    request->_tempObject = new String();
                }
                String* bodyPtr = (String*)request->_tempObject;
                if (bodyPtr) {
                    for(size_t i = 0; i < len; i++) {
                        *bodyPtr += (char)data[i];
                    }
                }
            }
        );
    };

    // Main Web Entry
    _server.on("/web", HTTP_GET, [this](AsyncWebServerRequest *request) { this->handlePortalRoot(request); });
    _server.on("/web/", HTTP_GET, [this](AsyncWebServerRequest *request) { this->handlePortalRoot(request); });
    _server.on("/web/index.html", HTTP_GET, [this](AsyncWebServerRequest *request) { this->handlePortalRoot(request); });

    // API: System & Config (Unified)
    _server.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest *request) { this->handleGetStatus(request); });
    _server.on("/api/config", HTTP_GET, [this](AsyncWebServerRequest *request) { this->handleGetConfig(request); });
    bindPostDynamic("/api/saveall", &WebHandlerManager::handleSaveAll);
    _server.on("/api/reboot", HTTP_POST, [this](AsyncWebServerRequest *request) { this->handleReboot(request); });
    _server.on("/api/ota/check", HTTP_POST, [this](AsyncWebServerRequest *request) { this->handleOTACheck(request); });

    // API: Individual CRUD (Legacy/Granular support)
    _server.on("/api/boards", HTTP_GET, [this](AsyncWebServerRequest *request) { this->handleGetBoards(request); });
    _server.on("/api/boards", HTTP_POST, [this](AsyncWebServerRequest *request) { this->handleSaveBoard(request); });
    _server.on("/api/boards", HTTP_DELETE, [this](AsyncWebServerRequest *request) { this->handleDeepDeleteBoard(request); });

    // API: Keys (CRUD)
    _server.on("/api/keys", HTTP_GET, [this](AsyncWebServerRequest *request) { this->handleGetKeys(request); });
    bindPostDynamic("/api/keys", &WebHandlerManager::handleSaveKey);
    _server.on("/api/keys", HTTP_DELETE, [this](AsyncWebServerRequest *request) { this->handleDeleteKey(request); });
    bindPostDynamic("/api/keys/test", &WebHandlerManager::handleTestKey);

    // API: WiFi (Bespoke Captive Portal)
    _server.on("/api/wifi/scan", HTTP_GET, [this](AsyncWebServerRequest *request) { this->handleWiFiScan(request); });
    bindPostDynamic("/api/wifi/test", &WebHandlerManager::handleWiFiTest);
    _server.on("/api/wifi/reset", HTTP_POST, [this](AsyncWebServerRequest *request) { this->handleWiFiReset(request); });

    // API: Subsystems
    _server.on("/stationpicker", HTTP_GET, [this](AsyncWebServerRequest *request) { this->handleStationPicker(request); });

    // API: Feeds & Weather Diagnostics
    _server.on("/api/feeds/test", HTTP_GET, [this](AsyncWebServerRequest *request) { this->handleTestFeed(request); });
    _server.on("/api/weather/test", HTTP_GET, [this](AsyncWebServerRequest *request) { this->handleTestWeather(request); });
    bindPostDynamic("/api/boards/test", &WebHandlerManager::handleTestBoard);
    _server.on("/rss.json", HTTP_GET, [this](AsyncWebServerRequest *request) { this->handleRSSJson(request); });

    // Captive Portal Redirect
    _server.onNotFound([this](AsyncWebServerRequest *request) { this->handleCaptivePortalRedirect(request); });

    LOG_INFO("WEB", "WebHandlerManager routes registered.");
}

/**
 * @brief Serves the main web index.html file (gzipped from flash).
 */
void WebHandlerManager::handlePortalRoot(AsyncWebServerRequest *request) {
    LOG_INFO("WEB_API", "Serving /web/index.html (gzipped)");
    sendGzipFlash(request, index_html_gz, index_html_gz_len, "text/html");
}

/**
 * @brief API Handler for GET /api/status. Returns system health and connectivity metrics.
 */
void WebHandlerManager::handleGetStatus(AsyncWebServerRequest *request) {
    // LOG_DEBUG("WEB_API", "GET /api/status called - returning system health");
    JsonDocument doc;
    doc["heap"] = ESP.getFreeHeap();
    doc["total_heap"] = ESP.getHeapSize();
    doc["max_alloc"] = ESP.getMaxAllocHeap();
    doc["temp"] = temperatureRead();
    doc["uptime"] = millis() / 1000;
    doc["rssi"] = WiFi.RSSI();
    doc["ssid"] = WiFi.SSID();
    doc["ip"] = WiFi.localIP().toString();
    doc["ap_mode"] = appContext.getWifiManager().getAPMode();
    doc["connected"] = (WiFi.status() == WL_CONNECTED);
    
    // Add Time Details
    char timeBuf[32];
    struct tm timeinfo = appContext.getTimeManager().getCurrentTime();
    strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", &timeinfo);
    doc["local_time"] = timeBuf;
    doc["timezone"] = _config.getConfig().timezone;
    
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
    request->send(200, "application/json", output);
}

/**
 * @brief API Handler for GET /api/config. Returns the unified project configuration as JSON.
 */
void WebHandlerManager::handleGetConfig(AsyncWebServerRequest *request) {
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
    system["noScrolling"] = config.noScrolling;
    system["fastRefresh"] = (config.apiRefreshRate == FASTDATAUPDATEINTERVAL);
    system["rssUrl"] = config.rssUrl;
    system["rssName"] = config.rssName;

    // --- Feeds (Modern UI) ---
    JsonObject feeds = doc["feeds"].to<JsonObject>();
    feeds["rss"] = config.rssUrl;
    feeds["weatherKeyId"] = config.weatherKeyId;

    // --- API Key Registry ---
    JsonArray keysArr = doc["keys"].to<JsonArray>();
    for (int i = 0; i < config.keyCount; i++) {
        JsonObject kObj = keysArr.add<JsonObject>();
        kObj["id"] = config.keys[i].id;
        kObj["label"] = config.keys[i].label;
        kObj["type"] = config.keys[i].type;
        kObj["token"] = ""; // Strictly empty for security
        kObj["tokenExists"] = (strlen(config.keys[i].token) > 0);
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
        b["brightness"] = bc.brightness;
        b["apiKeyId"] = bc.apiKeyId; // Added missing field
    }

    // --- WiFi Config (Secure) ---
    JsonObject wifi = doc["wifi"].to<JsonObject>();
    wifi["ssid"] = appContext.getWifiManager().getSSID();
    wifi["pass"] = ""; 
    wifi["passExists"] = appContext.getWifiManager().hasPassword();

    String output;
    serializeJson(doc, output);
    request->send(200, "application/json", output);
}

void WebHandlerManager::handleSaveAll(AsyncWebServerRequest *request, const String& body) {
    LOG_INFO("WEB_API", "POST /api/saveall called - processing unified save");


    LOG_DEBUG("WEB_API", String("Payload size: ") + body.length() + " bytes");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, body);
    if (error) {
        request->send(400, "application/json", "{\"status\":\"error\",\"msg\":\"Invalid JSON\"}");
        return;
    }

        Config& config = _config.getConfig();
        
        // --- Step 1: Update Global System Settings ---
        if (doc["system"].is<JsonObject>()) {
            JsonObject sys = doc["system"];
            if (sys["hostname"].is<const char*>()) strlcpy(config.hostname, sys["hostname"], sizeof(config.hostname));
            if (sys["timezone"].is<const char*>()) strlcpy(config.timezone, sys["timezone"], sizeof(config.timezone));
            if (sys["brightness"].is<int>()) config.brightness = sys["brightness"];
            if (sys["flip"].is<bool>()) config.flipScreen = sys["flip"];
            if (sys["dateEnabled"].is<bool>()) config.dateEnabled = sys["dateEnabled"];
            if (sys["noScrolling"].is<bool>()) config.noScrolling = sys["noScrolling"];
            if (sys["fastRefresh"].is<bool>()) config.apiRefreshRate = sys["fastRefresh"] ? FASTDATAUPDATEINTERVAL : DATAUPDATEINTERVAL;
        }

        // --- Step 2: Update Board Configurations (Full Replace) ---
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
                bc.brightness = b["brightness"] | -1;
                strlcpy(bc.apiKeyId, b["apiKeyId"] | "", sizeof(bc.apiKeyId));
            }
        }

        // --- Step 3: Update RSS and Weather Service Links ---
        if (doc["feeds"].is<JsonObject>()) {
            JsonObject f = doc["feeds"];
            if (f["rss"].is<const char*>()) {
                strlcpy(config.rssUrl, f["rss"], sizeof(config.rssUrl));
                config.rssEnabled = (strlen(config.rssUrl) > 0);
            }
            if (f["weatherKeyId"].is<const char*>()) {
                strlcpy(config.weatherKeyId, f["weatherKeyId"], sizeof(config.weatherKeyId));
            }
        }

        // --- Step 4: Process WiFi Connectivity Changes ---
        if (doc["wifi"].is<JsonObject>()) {
            JsonObject w = doc["wifi"];
            const char* ssid = w["ssid"] | "";
            const char* pass = w["pass"] | "";
            if (strlen(ssid) > 0) {
                LOG_INFO("WEB_API", "Received new WiFi credentials. Saving to wifi.json...");
                appContext.getWifiManager().updateWiFi(ssid, pass);
            }
        }

        // --- Step 5: Persist and Notify ---
        _config.save();

        request->send(200, "application/json", "{\"status\":\"ok\"}");
        
        _config.notifyConsumersToReapplyConfig();

        // If we were in AP mode, we should reboot to connect to the new network
        if (appContext.getWifiManager().getAPMode()) {
            LOG_INFO("WEB_API", "System in AP mode. rebooting in 2 seconds to apply new connectivity...");
            delay(2000);
            ESP.restart();
    }
}

void WebHandlerManager::handleGetBoards(AsyncWebServerRequest *request) {
    // Placeholder for Phase 2 implementation
    request->send(200, "application/json", "[]");
}

void WebHandlerManager::handleSaveBoard(AsyncWebServerRequest *request) {
    // Placeholder for Phase 2 implementation
    request->send(200, "application/json", "{\"status\":\"saved\"}");
}

void WebHandlerManager::handleDeepDeleteBoard(AsyncWebServerRequest *request) {
    // Placeholder for Phase 2 implementation
    request->send(200, "application/json", "{\"status\":\"deleted\"}");
}

void WebHandlerManager::handleGetKeys(AsyncWebServerRequest *request) {
    const Config& config = _config.getConfig();
    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();

    for (int i = 0; i < config.keyCount; i++) {
        JsonObject kObj = arr.add<JsonObject>();
        kObj["id"] = config.keys[i].id;
        kObj["label"] = config.keys[i].label;
        kObj["type"] = config.keys[i].type;
        kObj["token"] = "";
        kObj["tokenExists"] = (strlen(config.keys[i].token) > 0);
    }

    String output;
    serializeJson(doc, output);
    request->send(200, "application/json", output);
}

void WebHandlerManager::handleSaveKey(AsyncWebServerRequest *request, const String& body) {


    JsonDocument doc;
    deserializeJson(doc, body);
    
    ApiKey key;
    strlcpy(key.id, doc["id"] | "", sizeof(key.id));
    strlcpy(key.label, doc["label"] | "", sizeof(key.label));
    strlcpy(key.type, doc["type"] | "", sizeof(key.type));
    
    const char* token = doc["token"] | "";
    // Only update token if it's provided and not empty
    if (strlen(token) > 0) {
        strlcpy(key.token, token, sizeof(key.token));
    } else {
        // Find existing key to preserve token
        ApiKey* existing = _config.getKeyById(key.id);
        if (existing) {
            strlcpy(key.token, existing->token, sizeof(key.token));
        }
    }

    if (strlen(key.label) == 0 || strlen(key.type) == 0) {
        request->send(400, "application/json", "{\"status\":\"error\",\"msg\":\"Label and Type are required\"}");
        return;
    }

    _config.updateKey(key);
    request->send(200, "application/json", "{\"status\":\"ok\"}");
}

void WebHandlerManager::handleDeleteKey(AsyncWebServerRequest *request) {
    if (!request->hasArg("id")) {
        request->send(400, "application/json", "{\"status\":\"error\",\"msg\":\"Missing ID\"}");
        return;
    }

    _config.deleteKey(request->arg("id").c_str());
    request->send(200, "application/json", "{\"status\":\"ok\"}");
}

void WebHandlerManager::handleTestKey(AsyncWebServerRequest *request, const String& body) {
    LOG_DEBUG("WEB_API", String("Arguments received: ") + request->args());
    for (int i = 0; i < request->args(); i++) {
        LOG_DEBUG("WEB_API", "Arg [" + request->argName(i) + "]: " + request->arg(i));
    }



    JsonDocument doc;
    deserializeJson(doc, body);
    const char* type = doc["type"] | "";
    const char* token = doc["token"] | "";
    
    // If token is empty, use the real stored token (test existing)
    if (strlen(token) == 0) {
        ApiKey* existing = _config.getKeyById(doc["id"] | "");
        if (existing) {
            token = existing->token;
            LOG_DEBUG("WEB_API", "Using stored token for ID: " + String(doc["id"] | ""));
        } else {
            LOG_WARN("WEB_API", "Token empty and no key found for ID: " + String(doc["id"] | ""));
        }
    }

    if (token && strlen(token) > 0) {
        String tokenKey = String(token);
        LOG_DEBUG("WEB_API", "Testing token length: " + String(tokenKey.length()) + " Starts with: " + tokenKey.substring(0, 5) + "...");
    }

    bool success = false;
    String errorMsg = "Unsupported test type";

    if (strcmp(type, "owm") == 0) {
        LOG_INFO("WEB_API", "Testing OWM key...");
        WeatherStatus tempStatus;
        tempStatus.lat = 51.52;
        tempStatus.lon = -0.13;
        success = appContext.getWeather().updateWeather(tempStatus, nullptr, token);
        if (!success) errorMsg = appContext.getWeather().lastErrorMsg;
    } else if (strcmp(type, "rail") == 0) {
        LOG_INFO("WEB_API", "Testing National Rail key via unified test interface...");
        nationalRailDataSource ds;
        int updRes = ds.testConnection(token);
        LOG_INFO("WEB_API", "NR Test updRes: " + String(updRes));
        success = (updRes == UPD_SUCCESS);
        if (!success) errorMsg = ds.getLastErrorMsg();
    } else if (strcmp(type, "tfl") == 0) {
        LOG_INFO("WEB_API", "Testing TfL key via unified test interface...");
        tflDataSource ds;
        int updRes = ds.testConnection(token);
        LOG_INFO("WEB_API", "TfL Test updRes: " + String(updRes));
        success = (updRes == UPD_SUCCESS);
        if (!success) errorMsg = ds.getLastErrorMsg();
    } else if (strcmp(type, "bus") == 0) {
        LOG_INFO("WEB_API", "Testing Bus source via unified test interface...");
        busDataSource ds;
        int updRes = ds.testConnection(token);
        success = (updRes == UPD_SUCCESS);
        if (!success) errorMsg = ds.getLastErrorMsg();
    }

    LOG_INFO("WEB_API", String("Test result for ") + type + ": " + (success ? "SUCCESS" : "FAILED (" + errorMsg + ")"));
    
    JsonDocument res;
    res["status"] = success ? "ok" : "fail";
    if (!success) res["msg"] = errorMsg;
    
    String output;
    serializeJson(res, output);
    request->send(200, "application/json", output);
}

void WebHandlerManager::handleWiFiScan(AsyncWebServerRequest *request) {
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
    request->send(200, "application/json", output);
}

void WebHandlerManager::handleWiFiTest(AsyncWebServerRequest *request, const String& body) {
    LOG_INFO("WEB_API", "POST /api/wifi/test - Attempting async connection test...");
    


    JsonDocument doc;
    deserializeJson(doc, body);
    const char* ssid = doc["ssid"] | "";
    const char* pass = doc["pass"] | "";

    if (strlen(ssid) == 0) {
        request->send(400, "application/json", "{\"status\":\"error\",\"msg\":\"SSID required\"}");
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
    request->send(200, "application/json", output);
}

void WebHandlerManager::handleWiFiReset(AsyncWebServerRequest *request) {
    LOG_WARN("WEB_API", "POST /api/wifi/reset called - ERASING WiFi credentials and rebooting!");
    
    // 1. Erase credentials using WiFiConfig manager
    appContext.getWifiManager().resetSettings();

    // 2. Respond to client before rebooting
    request->send(200, "application/json", "{\"status\":\"ok\",\"msg\":\"WiFi erased. Board rebooting into AP mode...\"}");

    // 3. Trigger reboot after short delay to allow response to send
    LOG_INFO("SYSTEM", "Rebooting in 2 seconds...");
    delay(2000);
    ESP.restart();
}

void WebHandlerManager::handleCaptivePortalRedirect(AsyncWebServerRequest *request) {
    String host = request->host();
    LOG_INFO("WEB", String("Captive Portal: Redirecting request from ") + host + " to /web");
    
    // Redirect all requests that are NOT for our API/Portal to the web root
    request->redirect("/web");
}

void WebHandlerManager::handleTestFeed(AsyncWebServerRequest *request) {
    if (!request->hasArg("url")) {
        request->send(400, "application/json", "{\"status\":\"error\",\"msg\":\"Missing URL\"}");
        return;
    }
    String url = request->arg("url");
    LOG_INFO("WEB_API", "Testing RSS Feed: " + url);
    
    rssClient& rss = appContext.getRss();
    int res = rss.loadFeed(url);
    
    JsonDocument doc;
    doc["status"] = (res == 0) ? "ok" : "fail";
    if (res == 0) {
        doc["count"] = rss.numRssTitles;
        doc["title"] = rss.numRssTitles > 0 ? rss.rssTitle[0] : "No items found";
    } else {
        doc["msg"] = rss.getLastError();
    }
    
    String output;
    serializeJson(doc, output);
    request->send(200, "application/json", output);
}

void WebHandlerManager::handleTestWeather(AsyncWebServerRequest *request) {
    if (!request->hasArg("keyId")) {
        request->send(400, "application/json", "{\"status\":\"error\",\"msg\":\"Missing Key ID\"}");
        return;
    }
    String keyIdStr = request->arg("keyId");
    const char* keyId = keyIdStr.c_str();
    String tokenStr = request->arg("token");
    const char* token = request->hasArg("token") ? tokenStr.c_str() : nullptr;

    LOG_INFO("WEB_API", "Testing Weather Key: " + keyIdStr + (token ? " (with token override)" : " (stored)"));
    
    weatherClient& weather = appContext.getWeather();
    
    WeatherStatus ws;
    ws.lat = 51.7487; // Pen-y-darren Ironworks (Hardcoded for testing)
    ws.lon = -3.3816;
    
    LOG_INFO("WEB_API", "Testing Weather using coordinates: 51.7487° N, 3.3816° W (Pen-y-darren)");
    
    // Pass both. WeatherClient handles the logic of which to use.
    bool success = weather.updateWeather(ws, keyId, token);
    
    JsonDocument doc;
    doc["status"] = success ? "ok" : "fail";
    if (success) {
        doc["temp"] = (int)ws.temp;
        doc["condition"] = ws.description;
        doc["name"] = "Pen-y-darren Ironworks";
    } else {
        doc["msg"] = weather.lastErrorMsg;
    }
    
    String output;
    serializeJson(doc, output);
    LOG_INFO("WEB_API", "Weather test complete. Success=" + String(success ? "YES" : "NO") + " Result=" + output);
    request->send(200, "application/json", output);
}

void WebHandlerManager::handleTestBoard(AsyncWebServerRequest *request, const String& body) {
    LOG_DEBUG("WEB_API", "POST /api/boards/test - Arguments received: " + String(request->args()));
    


    JsonDocument doc;
    deserializeJson(doc, body);
    const char* typeStr = doc["type"] | "";
    const char* stationId = doc["id"] | "";
    const char* keyId = doc["apiKeyId"] | "";
    
    // For testing a board, we need the token from the associated key
    const char* token = nullptr;
    String tokenStr;
    if (strlen(keyId) > 0) {
        ApiKey* key = _config.getKeyById(keyId);
        if (key) {
            token = key->token;
        }
    }

    LOG_INFO("WEB_API", "Testing Board: Type=" + String(typeStr) + " Station=" + String(stationId) + " KeyID=" + String(keyId));

    bool success = false;
    String errorMsg = "Unsupported board type";
    int type = -1;

    if (strcmp(typeStr, "rail") == 0) type = 0;
    else if (strcmp(typeStr, "tfl") == 0) type = 1;
    else if (strcmp(typeStr, "bus") == 0) type = 2;
    else if (strcmp(typeStr, "clock") == 0) type = 3;

    if (type == 0) { // Rail
        nationalRailDataSource ds;
        int res = ds.testConnection(token, stationId);
        success = (res == UPD_SUCCESS || res == UPD_NO_CHANGE);
        if (!success) errorMsg = ds.getLastErrorMsg();
    } else if (type == 1) { // TfL
        tflDataSource ds;
        int res = ds.testConnection(token, stationId);
        success = (res == UPD_SUCCESS || res == UPD_NO_CHANGE);
        if (!success) errorMsg = ds.getLastErrorMsg();
    } else if (type == 2) { // Bus
        busDataSource ds;
        int res = ds.testConnection(token, stationId);
        success = (res == UPD_SUCCESS || res == UPD_NO_CHANGE);
        if (!success) errorMsg = ds.getLastErrorMsg();
    } else if (type == 3) { // Clock
        success = true; // Clock always works
    }

    LOG_INFO("WEB_API", String("Board test result: ") + (success ? "SUCCESS" : "FAILED (" + errorMsg + ")"));
    
    JsonDocument res;
    res["status"] = success ? "ok" : "fail";
    if (!success) res["msg"] = errorMsg;
    
    String output;
    serializeJson(res, output);
    request->send(200, "application/json", output);
}

/**
 * @brief API Handler for GET /rss.json. Serves the bundled RSS feed list.
 */
void WebHandlerManager::handleRSSJson(AsyncWebServerRequest *request) {
    // LOG_INFO("WEB_API", "Serving /rss.json (gzipped)");
    sendGzipFlash(request, rss_json_gz, rss_json_gz_len, "application/json");
}

/**
 * @brief API Handler for POST /api/reboot. Restarts the physical device.
 */
void WebHandlerManager::handleReboot(AsyncWebServerRequest *request) {
    LOG_WARN("WEB_API", "Reboot requested via API.");
    request->send(200, "application/json", "{\"status\":\"ok\"}");
    delay(1000);
    ESP.restart();
}

/**
 * @brief API Handler for POST /api/ota/check. Triggers a background firmware update check.
 */
void WebHandlerManager::handleOTACheck(AsyncWebServerRequest *request) {
    LOG_INFO("WEB_API", "Manual OTA update check requested.");
    request->send(200, "application/json", "{\"status\":\"ok\"}");
    
    // Trigger the update check in the background
    // We don't wait for it to finish before responding to the web UI
    appContext.getOtaUpdater().checkForFirmwareUpdate();
}

/**
 * @brief Helper to send gzipped data from flash memory with correct headers.
 * @param data Pointer to the gzipped data in flash.
 * @param len Length of the data in bytes.
 * @param contentType The MIME type to send.
 */
void WebHandlerManager::handleStationPicker(AsyncWebServerRequest *request) {
    if (!request->hasParam("q")) {
        request->send(400, "text/plain", "Missing Query");
        return;
    }

    String query = request->getParam("q")->value();
    if (query.length() <= 2) {
        request->send(400, "text/plain", "Query Too Short");
        return;
    }

    const char* host = "ojp.nationalrail.co.uk";
    WiFiClientSecure httpsClient;
    httpsClient.setInsecure();
    httpsClient.setTimeout(5000);

    int retryCounter = 0;
    while (!httpsClient.connect(host, 443) && retryCounter++ < 20) {
        delay(50);
    }

    if (retryCounter >= 20) {
        request->send(408, "text/plain", "NR Timeout");
        return;
    }

    httpsClient.print(String("GET /find/stationsDLRLU/") + query + " HTTP/1.0\r\n" +
                      "Host: ojp.nationalrail.co.uk\r\n" +
                      "Connection: close\r\n\r\n");

    retryCounter = 0;
    while (!httpsClient.available() && retryCounter++ < 15) {
        delay(100);
    }

    if (!httpsClient.available()) {
        httpsClient.stop();
        request->send(408, "text/plain", "NRQ Timeout");
        return;
    }

    String statusLine = httpsClient.readStringUntil('\n');
    statusLine.trim();

    if (!statusLine.startsWith("HTTP/") || statusLine.indexOf("200") == -1) {
        httpsClient.stop();
        if (statusLine.indexOf("401") > 0) request->send(401, "text/plain", "Not Authorized");
        else if (statusLine.indexOf("500") > 0) request->send(500, "text/plain", "Server Error");
        else request->send(503, "text/plain", statusLine.c_str());
        return;
    }

    while (httpsClient.connected() || httpsClient.available()) {
        String line = httpsClient.readStringUntil('\n');
        if (line == "\r" || line == "") break;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, httpsClient);
    httpsClient.stop();

    if (error) {
        request->send(500, "text/plain", "JSON Parse Error");
        return;
    }

    JsonDocument outDoc;
    JsonArray stationsArr = outDoc["payload"]["stations"].to<JsonArray>();

    if (doc.is<JsonArray>()) {
        for (JsonVariant item : doc.as<JsonArray>()) {
            if (item.is<JsonArray>()) {
                JsonArray stationData = item.as<JsonArray>();
                JsonObject stationObj = stationsArr.add<JsonObject>();
                stationObj["crsCode"] = stationData[0];
                stationObj["name"] = stationData[1];
                
                float lat = stationData[7].as<float>();
                float lon = stationData[8].as<float>();
                stationObj["latitude"] = lat;
                stationObj["longitude"] = lon;
                stationObj["kbState"] = 1;
            }
        }
    }

    String output;
    serializeJson(outDoc, output);
    request->send(200, "application/json", output);
}

void WebHandlerManager::sendGzipFlash(AsyncWebServerRequest *request, const uint8_t* data, size_t len, const char* contentType) {
    AsyncWebServerResponse *response = request->beginResponse(200, contentType, data, len);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
}
