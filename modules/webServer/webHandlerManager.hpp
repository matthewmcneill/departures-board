/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/webServer/webHandlerManager.hpp
 * Description: Decoupled manager for modern web portal endpoints and assets.
 *
 * Exported Functions/Classes:
 * - WebHandlerManager: Class for registering and handling portal-specific HTTP routes.
 *   - begin(): Registers all routes with the ESP32 WebServer.
 *   - handlePortalRoot(): Serves gzipped SPA assets from PROGMEM.
 *   - handleGetStatus(): Returns JSON representing hardware health.
 *   - handleGetConfig(): Returns unified System/Keys/Boards configuration.
 *   - handleSaveAll(): Atomic validation and save of portal settings.
 *   - handleWiFiScan(): Returns JSON list of local SSIDs/RSSI.
 *   - handleWiFiTest(): Asynchronous non-blocking connection verification.
 *   - handleWiFiReset(): Erases NVS/LittleFS credentials and reboots to AP.
 *   - handleRSSJson(): Serves the bundled JSON list of RSS feeds.
 *   - handleReboot(): API handler to restart the physical device.
 *   - handleOTACheck(): API handler to trigger a manual check for firmware updates.
 */

#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <configManager.hpp>

/**
 * @brief Manages the registration and execution of web handlers for the modern portal.
 */
class WebHandlerManager {
public:
    /**
     * @brief Constructor for WebHandlerManager.
     * @param server Reference to the active AsyncWebServer.
     * @param config Reference to the ConfigManager.
     */
    WebHandlerManager(AsyncWebServer& server, ConfigManager& config);

    /**
     * @brief Register all portal routes with the server.
     */
    void begin();

private:
    AsyncWebServer& _server;
    ConfigManager& _config;

    // --- Route Handlers ---
    
    void handlePortalRoot(AsyncWebServerRequest *request);
    void handleGetStatus(AsyncWebServerRequest *request);
    void handleSaveConfig(AsyncWebServerRequest *request);
    void handleGetConfig(AsyncWebServerRequest *request);
    void handleSaveAll(AsyncWebServerRequest *request, const String& body);
    void handleWiFiScan(AsyncWebServerRequest *request);
    void handleWiFiTest(AsyncWebServerRequest *request, const String& body);
    void handleWiFiReset(AsyncWebServerRequest *request);
    void handleGetBoards(AsyncWebServerRequest *request);
    void handleSaveBoard(AsyncWebServerRequest *request);
    void handleDeepDeleteBoard(AsyncWebServerRequest *request);
    void handleGetKeys(AsyncWebServerRequest *request);
    void handleSaveKey(AsyncWebServerRequest *request, const String& body);
    void handleDeleteKey(AsyncWebServerRequest *request);
    void handleTestKey(AsyncWebServerRequest *request, const String& body);
    void handleTestFeed(AsyncWebServerRequest *request);
    void handleTestWeather(AsyncWebServerRequest *request);
    void handleRSSJson(AsyncWebServerRequest *request);
    void handleTestBoard(AsyncWebServerRequest *request, const String& body);
    void handleCaptivePortalRedirect(AsyncWebServerRequest *request);
    void handleReboot(AsyncWebServerRequest *request);
    void handleOTACheck(AsyncWebServerRequest *request);
    void handleStationPicker(AsyncWebServerRequest *request);
    void handleSetDiagMode(AsyncWebServerRequest *request);

    // --- Helpers ---
    void sendGzipFlash(AsyncWebServerRequest *request, const uint8_t* data, size_t len, const char* contentType);
};
