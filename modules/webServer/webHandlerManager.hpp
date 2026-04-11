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
 * - WebHandlerManager: [Class] Service lifecycle for the web portal.
 *   - begin(): Route registration.
 *   - handlePortalRoot(): SPA delivery.
 *   - handleSaveAll(): Atomic config persistence.
 *   - handleWiFiScan(): Network discovery endpoint.
 *   - handleGetStatus(): Real-time telemetry provider.
 */

#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
class appContext;

/**
 * @brief Manages the registration and execution of web handlers for the modern portal.
 */
class WebHandlerManager {
public:
    /**
     * @brief Constructor for WebHandlerManager.
     * @param server Reference to the active AsyncWebServer.
     * @param context Reference to the global appContext.
     */
    WebHandlerManager(AsyncWebServer& server, appContext& context);

    /**
     * @brief Register all portal routes with the server.
     */
    void begin();

private:
    AsyncWebServer& _server; // Reference to the underlying network server
    appContext& _context;    // Reference to the central application context

    // --- Route Handlers ---
    
    void handlePortalRoot(AsyncWebServerRequest *request);
    void handleGetStatus(AsyncWebServerRequest *request);
    void handleSaveConfig(AsyncWebServerRequest *request);
    void handleGetConfig(AsyncWebServerRequest *request);
    void handleSaveAll(AsyncWebServerRequest *request, const String& body);
    void handleWiFiScan(AsyncWebServerRequest *request);
    void handleWiFiTest(AsyncWebServerRequest *request, const String& body);
    void handleWiFiReset(AsyncWebServerRequest *request);
    void handleBackupConfig(AsyncWebServerRequest *request);
    void handleRestoreConfig(AsyncWebServerRequest *request, const String& body);
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
    void handleOtaAvailable(AsyncWebServerRequest *request);
    void handleOtaForce(AsyncWebServerRequest *request);
    void handleOtaRollback(AsyncWebServerRequest *request);
    void handleStationPicker(AsyncWebServerRequest *request);
    void handleSetDiagMode(AsyncWebServerRequest *request);
    void handleScreenshot(AsyncWebServerRequest *request);
    void handleMcpRequest(AsyncWebServerRequest *request, const String& body);

    // --- Helpers ---
    void sendGzipFlash(AsyncWebServerRequest *request, const uint8_t* data, size_t len, const char* contentType);
};
