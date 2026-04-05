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
 * - WebHandlerManager: [Class implementation]
 *   - begin: Strategic route registration.
 *   - handlePortalRoot: High-memory delivery of SPA assets.
 *   - handleGetStatus: Telemetry serialization.
 *   - handleSaveAll: Atomic configuration engine.
 */

#include "webHandlerManager.hpp"
// Web UI Assets Embed
#include "portalAssets.h"
#include "../../src/departuresBoard.hpp"
#include "../../src/buildTime.hpp"
#include <ArduinoJson.h>
#include <logger.hpp>
#include <wifiManager.hpp>
#include <WiFi.h>
#include <esp_task_wdt.h>
#include <appContext.hpp>
#include "../displayManager/boards/nationalRailBoard/nrDARWINDataProvider.hpp"
#include "../displayManager/boards/nationalRailBoard/nrRDMDataProvider.hpp"
#include "../displayManager/boards/tflBoard/tflDataSource.hpp"
#include "../displayManager/boards/busBoard/busDataSource.hpp"
#include "../../lib/rssClient/rssClient.hpp"
#include "../weatherClient/weatherClient.hpp"
#include <memory>
#include <atomic>

extern class appContext appContext;

// Static atomic counter to track active high-memory web requests (e.g. serving 152KB HTML or large JSON)
// This serializes delivery to protect the 34KB max heap block limit from concurrent mobile connections.
static std::atomic<int> activeHighMemRequests{0}; // Shared atomic for thread-safe session tracking
const int MAX_CONCURRENT_HIGH_MEM = 1; // Limit for memory-intensive web responses

/**
 * @brief Constructor for the WebHandlerManager.
 * @param server Reference to the AsyncWebServer.
 * @param context Reference to the global appContext.
 */
WebHandlerManager::WebHandlerManager(AsyncWebServer& server, class appContext& context) 
    : _server(server), _context(context) {
}

/**
 * @brief Registers all web portal routes with the underlying server.
 * Implements a mix of static asset delivery and dynamic JSON API endpoints.
 * Includes security-sensitive POST body reconstruction helper.
 */
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
    _server.on("/screenshot.html", HTTP_GET, [this](AsyncWebServerRequest *request) { 
        this->sendGzipFlash(request, screenshot_html_gz, screenshot_html_gz_len, "text/html"); 
    });

    // API: System & Config (Unified)
    _server.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest *request) { this->handleGetStatus(request); });
    _server.on("/api/config", HTTP_GET, [this](AsyncWebServerRequest *request) { this->handleGetConfig(request); });
    bindPostDynamic("/api/saveall", &WebHandlerManager::handleSaveAll);
    _server.on("/api/reboot", HTTP_POST, [this](AsyncWebServerRequest *request) { this->handleReboot(request); });
    _server.on("/api/ota/check", HTTP_POST, [this](AsyncWebServerRequest *request) { this->handleOTACheck(request); });
    _server.on("/api/system/diag", HTTP_POST, [this](AsyncWebServerRequest *request) { this->handleSetDiagMode(request); });
    _server.on("/api/screenshot", HTTP_GET, [this](AsyncWebServerRequest *request) { this->handleScreenshot(request); });
    _server.on("/api/config/backup", HTTP_GET, [this](AsyncWebServerRequest *request) { this->handleBackupConfig(request); });
    bindPostDynamic("/api/config/restore", &WebHandlerManager::handleRestoreConfig);

    // API: Feeds & Weather Diagnostics (More specific routes MUST come first in ESPAsyncWebServer)
    _server.on("/api/feeds/test", HTTP_GET, [this](AsyncWebServerRequest *request) { this->handleTestFeed(request); });
    _server.on("/api/weather/test", HTTP_GET, [this](AsyncWebServerRequest *request) { this->handleTestWeather(request); });
    bindPostDynamic("/api/boards/test", &WebHandlerManager::handleTestBoard);
    bindPostDynamic("/api/keys/test", &WebHandlerManager::handleTestKey);
    bindPostDynamic("/api/wifi/test", &WebHandlerManager::handleWiFiTest);

    // API: Individual CRUD (Legacy/Granular support)
    _server.on("/api/boards", HTTP_GET, [this](AsyncWebServerRequest *request) { this->handleGetBoards(request); });
    _server.on("/api/boards", HTTP_POST, [this](AsyncWebServerRequest *request) { this->handleSaveBoard(request); });
    _server.on("/api/boards", HTTP_DELETE, [this](AsyncWebServerRequest *request) { this->handleDeepDeleteBoard(request); });

    // API: Keys (CRUD)
    _server.on("/api/keys", HTTP_GET, [this](AsyncWebServerRequest *request) { this->handleGetKeys(request); });
    bindPostDynamic("/api/keys", &WebHandlerManager::handleSaveKey);
    _server.on("/api/keys", HTTP_DELETE, [this](AsyncWebServerRequest *request) { this->handleDeleteKey(request); });

    // API: WiFi (Bespoke Captive Portal)
    _server.on("/api/wifi/scan", HTTP_GET, [this](AsyncWebServerRequest *request) { this->handleWiFiScan(request); });
    _server.on("/api/wifi/reset", HTTP_POST, [this](AsyncWebServerRequest *request) { this->handleWiFiReset(request); });

    // API: Subsystems
    _server.on("/stationpicker", HTTP_GET, [this](AsyncWebServerRequest *request) { this->handleStationPicker(request); });

    _server.on("/rss.json", HTTP_GET, [this](AsyncWebServerRequest *request) { this->handleRSSJson(request); });

    // Captive Portal Redirect
    _server.onNotFound([this](AsyncWebServerRequest *request) { this->handleCaptivePortalRedirect(request); });

    LOG_INFO("WEB", "WebHandlerManager routes registered.");
}

/**
 * @brief Serves the main web index.html file (gzipped from flash).
 */
void WebHandlerManager::handlePortalRoot(AsyncWebServerRequest *request) {
    char logBuf[128];
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t maxBlock = ESP.getMaxAllocHeap();
    
    snprintf(logBuf, sizeof(logBuf), "Serving /web/index.html (gzipped) | Heap: %u | MaxBlock: %u", freeHeap, maxBlock);
    LOG_INFO("WEB_API", logBuf);

    // Serialization Guard: If another high-memory request is active, or heap is dangerously low, 
    // reject the request to prevent a system crash. Mobile browsers often open multiple 
    // concurrent sockets which would otherwise exhaust the 34KB heap blocks.
    if (activeHighMemRequests >= MAX_CONCURRENT_HIGH_MEM || maxBlock < 20000) {
        LOG_WARN("WEB_API", "Heavy request rejected (Heap/Concurrency Guard)");
        request->send(503, "text/plain", "Busy - Retry in 1s");
        return;
    }

    activeHighMemRequests++;
    
    // Create the response and decrement the counter when it is finished/closed
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", index_html_gz, index_html_gz_len);
    response->addHeader("Content-Encoding", "gzip");
    
    // Decoupled decrement: The response object is destroyed when the request is finalized.
    // We utilize the onDisconnect callback to ensure the semaphore is released safely.
    request->onDisconnect([]() {
        if (activeHighMemRequests > 0) activeHighMemRequests--;
    });

    request->send(response);
}

/**
 * @brief API Handler for GET /api/status. Returns system health and connectivity metrics.
 * Provides real-time telemetry for the Web UI dashboard.
 * @param request Pointer to the incoming AsyncWebServerRequest.
 */
void WebHandlerManager::handleGetStatus(AsyncWebServerRequest *request) {
    // LOG_DEBUG("WEB_API", "GET /api/status called - returning system health");
    JsonDocument doc;
    doc["heap"] = ESP.getFreeHeap();
    doc["total_heap"] = ESP.getHeapSize();
    doc["max_alloc"] = ESP.getMaxAllocHeap();
    doc["min_heap"] = ESP.getMinFreeHeap();
    doc["temp"] = temperatureRead();
    doc["uptime"] = millis() / 1000;
    doc["build"] = BUILD_TIME;
    doc["rssi"] = WiFi.RSSI();
    doc["ssid"] = WiFi.SSID();
    doc["ip"] = WiFi.localIP().toString();
    doc["ap_mode"] = appContext.getWifiManager().getAPMode();
    doc["connected"] = (WiFi.status() == WL_CONNECTED);
    
    char vBuf[16];
    snprintf(vBuf, sizeof(vBuf), "v%d.%d", VERSION_MAJOR, VERSION_MINOR);
    doc["version"] = vBuf;
    
    // Add Time Details
    char timeBuf[32];
    struct tm timeinfo = appContext.getTimeManager().getCurrentTime();
    strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", &timeinfo);
    doc["local_time"] = timeBuf;
    doc["timezone"] = _context.getConfigManager().getConfig().timezone;
    
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
    char logBuf[128];
    uint32_t maxBlock = ESP.getMaxAllocHeap();
    snprintf(logBuf, sizeof(logBuf), "GET /api/config | MaxBlock: %u", maxBlock);
    LOG_INFO("WEB_API", logBuf);

    // Serialization Guard: JSON generation for the full config is memory-intensive.
    // If another high-memory task is running, wait for it to protect the heap.
    if (activeHighMemRequests >= MAX_CONCURRENT_HIGH_MEM || maxBlock < 18000) {
        LOG_WARN("WEB_API", "Config request rejected (Heap/Concurrency Guard)");
        request->send(503, "text/plain", "Busy - Retry in 1s");
        return;
    }

    activeHighMemRequests++;
    
    // Ensure the semaphore is released even if the connection is terminated prematurely.
    request->onDisconnect([]() {
        if (activeHighMemRequests > 0) activeHighMemRequests--;
    });

    const Config& config = _context.getConfigManager().getConfig();
    JsonDocument doc;

    doc["version"] = config.configVersion; // Expose schema version directly at the root for UI tracking

    // --- System Settings ---
    JsonObject system = doc["system"].to<JsonObject>();
    system["hostname"] = config.hostname;
    system["timezone"] = config.timezone;
    system["brightness"] = config.brightness;
    system["flip"] = config.flipScreen;
    system["dateEnabled"] = config.dateEnabled;
    system["noScrolling"] = config.noScrolling;
    system["fastRefresh"] = (config.apiRefreshRate == FASTDATAUPDATEINTERVAL);
    system["overrideTimeout"] = config.manualOverrideTimeoutSecs;
    system["carouselInterval"] = config.carouselIntervalSecs;
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

    // --- Schedules ---
    JsonArray schedArr = doc["schedules"].to<JsonArray>();
    for (int i = 0; i < MAX_SCHEDULE_RULES; i++) {
        const ScheduleRule& r = config.schedules[i];
        JsonObject s = schedArr.add<JsonObject>();
        s["startH"] = r.startHour;
        s["startM"] = r.startMinute;
        s["endH"] = r.endHour;
        s["endM"] = r.endMinute;
        s["board"] = r.boardIndex;
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
        b["apiKeyId"] = bc.apiKeyId;
        b["tflLine"] = bc.tflLineFilter;
        b["tflDir"] = bc.tflDirectionFilter;
        b["ordinals"] = bc.showServiceOrdinals;
        b["lastSeen"] = bc.showLastSeenLocation;
        b["oledOff"] = bc.oledOff;
        b["layout"] = bc.layout;
    }

    // --- WiFi Config (Secure) ---
    JsonObject wifi = doc["wifi"].to<JsonObject>();
    wifi["ssid"] = appContext.getWifiManager().getSSID();
    wifi["pass"] = ""; 
    wifi["passExists"] = appContext.getWifiManager().hasPassword();

    // --- System Limits (Dynamic Export) ---
    JsonObject limits = doc["limits"].to<JsonObject>();
    limits["MAX_BOARDS"] = MAX_BOARDS;
    limits["MAX_KEYS"] = MAX_KEYS;
    limits["MAX_SCHEDULE_RULES"] = MAX_SCHEDULE_RULES;

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

        Config& config = _context.getConfigManager().getConfig();
        
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
            if (sys["overrideTimeout"].is<int>()) config.manualOverrideTimeoutSecs = sys["overrideTimeout"];
            if (sys["carouselInterval"].is<int>()) config.carouselIntervalSecs = sys["carouselInterval"];
        }

        // --- Step 2: Update Board Configurations (Full Replace) ---
        if (doc["boards"].is<JsonArray>()) {
            JsonArray boardsArr = doc["boards"];
            config.boardCount = 0;
            for (JsonObject b : boardsArr) {
                if (config.boardCount >= MAX_BOARDS) break;
                BoardConfig& bc = config.boards[config.boardCount++];
                bc.type = (BoardTypes)(b["type"] | 0);
                strlcpy(bc.id, b["id"].as<String>().c_str(), sizeof(bc.id));
                strlcpy(bc.name, b["name"].as<String>().c_str(), sizeof(bc.name));
                bc.lat = b["lat"] | 0.0f;
                bc.lon = b["lon"] | 0.0f;
                strlcpy(bc.filter, b["filter"].as<String>().c_str(), sizeof(bc.filter));
                strlcpy(bc.secondaryId, b["secId"].as<String>().c_str(), sizeof(bc.secondaryId));
                strlcpy(bc.secondaryName, b["secName"].as<String>().c_str(), sizeof(bc.secondaryName));
                bc.timeOffset = b["offset"] | 0;
                bc.showWeather = b["weather"] | true;
                bc.brightness = b["brightness"] | -1;
                strlcpy(bc.apiKeyId, b["apiKeyId"].as<String>().c_str(), sizeof(bc.apiKeyId));
                strlcpy(bc.tflLineFilter, b["tflLine"].as<String>().c_str(), sizeof(bc.tflLineFilter));
                bc.tflDirectionFilter = b["tflDir"] | 0;
                bc.showServiceOrdinals = b["ordinals"] | false;
                bc.showLastSeenLocation = b["lastSeen"] | false;
                bc.oledOff = b["oledOff"] | false;
                strlcpy(bc.layout, b["layout"].as<String>().c_str(), sizeof(bc.layout));
            }
        }

        // --- Step 2.5: Update Schedules (Full Replace) ---
        if (doc["schedules"].is<JsonArray>()) {
            JsonArray schedArr = doc["schedules"];
            int count = 0;
            for (JsonObject s : schedArr) {
                if (count >= MAX_SCHEDULE_RULES) break;
                ScheduleRule& r = config.schedules[count++];
                r.startHour = s["startH"] | 0;
                r.startMinute = s["startM"] | 0;
                r.endHour = s["endH"] | 23;
                r.endMinute = s["endM"] | 59;
                r.boardIndex = s["board"] | -1;
            }
            // Fill remaining with defaults
            for (int i = count; i < MAX_SCHEDULE_RULES; i++) {
                config.schedules[i].boardIndex = -1;
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
        _context.getConfigManager().save();

        request->send(200, "application/json", "{\"status\":\"ok\"}");
        
        _context.getConfigManager().requestReload();

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
    const Config& config = _context.getConfigManager().getConfig();
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
        ApiKey* existing = _context.getConfigManager().getKeyById(key.id);
        if (existing) {
            strlcpy(key.token, existing->token, sizeof(key.token));
        }
    }

    if (strlen(key.label) == 0 || strlen(key.type) == 0) {
        request->send(400, "application/json", "{\"status\":\"error\",\"msg\":\"Label and Type are required\"}");
        return;
    }

    _context.getConfigManager().updateKey(key);
    request->send(200, "application/json", "{\"status\":\"ok\"}");
}

void WebHandlerManager::handleDeleteKey(AsyncWebServerRequest *request) {
    if (!request->hasArg("id")) {
        request->send(400, "application/json", "{\"status\":\"error\",\"msg\":\"Missing ID\"}");
        return;
    }

    _context.getConfigManager().deleteKey(request->arg("id").c_str());
    request->send(200, "application/json", "{\"status\":\"ok\"}");
}

struct ApiTestParams {
    String type;
    String token;
    String station;
    volatile bool done;
    String jsonObj;
};

class ApiTestDataSource : public iDataSource {
public:
    std::unique_ptr<ApiTestParams> params;
    uint32_t nextFetch = 0;
    
    ApiTestDataSource(std::unique_ptr<ApiTestParams> p) : params(std::move(p)) {}
    void executeFetch() override {
        bool success = false; String errorMsg = "Unsupported test type";

        if (params->type == "owm") {
            WeatherStatus tempStatus; tempStatus.lat = 51.52; tempStatus.lon = -0.13;
            success = appContext.getWeather().updateWeather(tempStatus, nullptr, params->token.c_str());
            if (!success) errorMsg = appContext.getWeather().lastErrorMsg;
        } else if (params->type == "rail") {
            nrDARWINDataProvider ds;
            success = (ds.testConnection(params->token.c_str(), params->station.length() > 0 ? params->station.c_str() : "PAD") == UpdateStatus::SUCCESS);
            if (!success) errorMsg = ds.getLastErrorMsg();
        } else if (params->type == "rdm") {
            nrRDMDataProvider ds;
            success = (ds.testConnection(params->token.c_str(), params->station.length() > 0 ? params->station.c_str() : "PAD") == UpdateStatus::SUCCESS);
            if (!success) errorMsg = ds.getLastErrorMsg();
        } else if (params->type == "tfl") {
            tflDataSource ds;
            UpdateStatus res = ds.testConnection(params->token.c_str(), params->station.length() > 0 ? params->station.c_str() : nullptr);
            success = (res == UpdateStatus::SUCCESS || res == UpdateStatus::NO_CHANGE);
            if (!success) errorMsg = ds.getLastErrorMsg();
        } else if (params->type == "bus") {
            busDataSource ds;
            UpdateStatus res = ds.testConnection(params->token.c_str(), params->station.length() > 0 ? params->station.c_str() : nullptr);
            success = (res == UpdateStatus::SUCCESS || res == UpdateStatus::NO_CHANGE);
            if (!success) errorMsg = ds.getLastErrorMsg();
        } else if (params->type == "clock") {
            success = true;
        }

        JsonDocument res; res["status"] = success ? "ok" : "fail";
        if (!success) res["msg"] = errorMsg;
        serializeJson(res, params->jsonObj);
        
        params->done = true;
    }
    UpdateStatus updateData() override { return UpdateStatus::SUCCESS; }
    UpdateStatus testConnection(const char*, const char*) override { return UpdateStatus::SUCCESS; }
    const char* getLastErrorMsg() const override { return ""; }
    uint32_t getNextFetchTime() override { return nextFetch; }
    PriorityTier getPriorityTier() override { return PriorityTier::PRIO_CRITICAL; } // Test connections must run immediately
    void setNextFetchTime(uint32_t t) override { nextFetch = t; }
};

void WebHandlerManager::handleTestKey(AsyncWebServerRequest *request, const String& body) {
    JsonDocument doc;
    deserializeJson(doc, body);
    String typeStr = doc["type"] | "";
    String tokenStr = doc["token"] | "";
    
    if (tokenStr.isEmpty()) {
        ApiKey* existing = _context.getConfigManager().getKeyById(doc["id"] | "");
        if (existing) tokenStr = existing->token;
    }

    try {
        auto params = std::make_unique<ApiTestParams>();
        params->type = typeStr;
        params->token = tokenStr;
        params->done = false;

        // Transfer unique ownership to the DataSource.
        // After std::move, params is guaranteed to be nullptr for safety.
        auto testSource = std::make_unique<ApiTestDataSource>(std::move(params));

        appContext.getDataManager().registerSource(testSource.get());
        appContext.getDataManager().requestPriorityFetch(testSource.get());

        // Yield gracefully until the Background DataManager payload finishes, with absolute 10s timeout
        int maxWait = 10000 / 50;
        int cycles = 0;
        
        // Pointer to the params inside the testSource for the wait loop
        ApiTestParams* pRef = testSource->params.get();

        while(!pRef->done && cycles < maxWait) { 
            // Feed the FreeRTOS Task Watchdog Timer. This loop executes on the async_tcp thread (Core 1).
            // Since ESPAsyncWebServer handlers are synchronous, waiting 10s without returning 
            // to the event loop will trigger a TWDT panic (default 5s) if we don't manually check in.
            esp_task_wdt_reset(); 
            vTaskDelay(pdMS_TO_TICKS(50)); 
            cycles++; 
        }

        if (cycles >= maxWait) {
            request->send(408, "application/json", "{\"status\":\"error\",\"msg\":\"Timeout!\"}");
        } else {
            request->send(200, "application/json", pRef->jsonObj);
        }
        
        appContext.getDataManager().unregisterSource(testSource.get());
    } catch (const std::bad_alloc& e) {
        LOG_ERROR("SYSTEM", "CRITICAL OOM: Failed to allocate TestDataSource on heap!");
        request->send(500, "application/json", "{\"status\":\"error\",\"msg\":\"Out of Memory on device\"}");
    }
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
    UpdateStatus res = rss.loadFeed(url);
    
    JsonDocument doc;
    doc["status"] = (res == UpdateStatus::SUCCESS || res == UpdateStatus::NO_CHANGE) ? "ok" : "fail";
    if (res == UpdateStatus::SUCCESS || res == UpdateStatus::NO_CHANGE) {
        doc["count"] = rss.numRssTitles;
        doc["title"] = rss.numRssTitles > 0 ? String(rss.rssTitle[0]) : "No items found";
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
    
    bool success = weather.updateWeather(ws, keyId, token);
    
    if (success) {
        int maxWait = 12000 / 50; // 12 seconds max
        int cycles = 0;
        
        while(weather.isFetchPending() && cycles < maxWait) { 
            esp_task_wdt_reset();
            vTaskDelay(pdMS_TO_TICKS(50)); 
            cycles++; 
        }

        if (weather.isFetchPending()) {
            success = false;
            strcpy(weather.lastErrorMsg, "Timeout waiting for queue");
        } else {
            success = (ws.status == WeatherUpdateStatus::READY);
        }
    }
    
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
    JsonDocument doc;
    deserializeJson(doc, body);
    String typeStr = doc["type"] | "";
    String stationId = doc["id"] | "";
    String keyId = doc["apiKeyId"] | "";
    
    String tokenStr = "";
    if (keyId.length() > 0) {
        ApiKey* key = _context.getConfigManager().getKeyById(keyId.c_str());
        if (key) tokenStr = key->token;
    }

    auto params = std::make_unique<ApiTestParams>();
    params->type = typeStr;
    params->token = tokenStr;
    params->station = stationId;
    params->done = false;

    // Transfer unique ownership to the DataSource.
    // After std::move, params is guaranteed to be nullptr for safety.
    auto testSource = std::make_unique<ApiTestDataSource>(std::move(params));

    // Hand execution to DataManager Core 0 queue to shield from OOM concurrent TLS
    appContext.getDataManager().registerSource(testSource.get());
    appContext.getDataManager().requestPriorityFetch(testSource.get());

    int maxWait = 10000 / 50;
    int cycles = 0;
    
    // Pointer to the params inside the testSource for the wait loop
    ApiTestParams* pRef = testSource->params.get();

    while(!pRef->done && cycles < maxWait) { 
        // Feed the FreeRTOS Task Watchdog Timer. This loop executes on the async_tcp thread (Core 1).
        // Since ESPAsyncWebServer handlers are synchronous, waiting 10s without returning 
        // to the event loop will trigger a TWDT panic (default 5s) if we don't manually check in.
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(50)); 
        cycles++; 
    }
    
    if (!pRef->done) {
        request->send(503, "application/json", "{\"status\":\"error\",\"msg\":\"Validation Queue Full or Timeout\"}");
        appContext.getDataManager().unregisterSource(testSource.get());
        // Smart pointers automatically handle cleanup
        return;
    }

    request->send(200, "application/json", pRef->jsonObj);
    appContext.getDataManager().unregisterSource(testSource.get());
    // Smart pointers automatically handle cleanup
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
 * @brief API Handler for POST /api/system/diag. Toggles the volatile diagnostic mode.
 */
void WebHandlerManager::handleSetDiagMode(AsyncWebServerRequest *request) {
    if (request->hasParam("active")) {
        bool active = request->getParam("active")->value() == "true";
        LOG_INFO("WEB_API", String("Diagnostic mode requested: ") + (active ? "ON" : "OFF"));
        appContext.getDisplayManager().setDiagMode(active);
        request->send(200, "application/json", "{\"status\":\"ok\"}");
    } else {
        request->send(400, "application/json", "{\"status\":\"error\",\"msg\":\"Missing 'active' param\"}");
    }
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

void WebHandlerManager::handleScreenshot(AsyncWebServerRequest *request) {
    LOG_INFO("WEB_API", "GET /api/screenshot called - capturing raw hardware display buffer");
    
    size_t bufferSize = _context.getDisplayManager().getFramebufferSize();
    const uint8_t* buffer = _context.getDisplayManager().getRawFramebuffer();
    
    if (buffer == nullptr || bufferSize == 0) {
        request->send(500, "text/plain", "Hardware buffer unavailable");
        return;
    }

    // Deliver the raw memory block exactly as it looks in SSD1322 native space
    AsyncWebServerResponse *response = request->beginResponse(200, "application/octet-stream", buffer, bufferSize);
    // Explicitly prevent any caching
    response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    response->addHeader("Pragma", "no-cache");
    response->addHeader("Expires", "0");
    request->send(response);
}

/**
 * @brief Serves the current config.json as a downloadable attachment.
 */
void WebHandlerManager::handleBackupConfig(AsyncWebServerRequest *request) {
    LOG_INFO("WEB_API", "GET /api/config/backup called");
    String configData = _context.getConfigManager().loadFile("/config.json");
    if (configData.length() == 0) {
        request->send(404, "text/plain", "Config not found");
        return;
    }
    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", configData);
    response->addHeader("Content-Disposition", "attachment; filename=config.json");
    request->send(response);
}

/**
 * @brief Receives a JSON configuration file and overwrites the local /config.json.
 * Validates that the payload is valid JSON before persisting.
 */
void WebHandlerManager::handleRestoreConfig(AsyncWebServerRequest *request, const String& body) {
    LOG_INFO("WEB_API", "POST /api/config/restore called");
    if (body.length() == 0) {
        request->send(400, "application/json", "{\"status\":\"error\",\"msg\":\"Empty body\"}");
        return;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, body);
    if (error || !doc.is<JsonObject>()) {
        request->send(400, "application/json", "{\"status\":\"error\",\"msg\":\"Invalid JSON format\"}");
        return;
    }
    
    // Guard against users uploading the live `/api/config` state dump instead of a valid backup.
    // The live dump contains nested "system", "wifi", and "feeds" objects which are too large
    // for standard deserialization during boot and will permanently corrupt the file system pipeline.
    if (doc.containsKey("system") || doc.containsKey("wifi")) {
        request->send(400, "application/json", "{\"status\":\"error\",\"msg\":\"Invalid format. Cannot restore from a live state dump. Please upload a valid Backup configuration file.\"}");
        return;
    }

    // Save the raw string to /config.json
    if (_context.getConfigManager().saveFile("/config.json", body)) {
        LOG_INFO("WEB_API", "Restored config.json successfully. Requesting reload.");
        _context.getConfigManager().requestReload();
        request->send(200, "application/json", "{\"status\":\"ok\"}");
    } else {
        request->send(500, "application/json", "{\"status\":\"error\",\"msg\":\"Failed to save file\"}");
    }
}
