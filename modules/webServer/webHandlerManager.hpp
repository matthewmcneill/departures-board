/*
 * Departures Board (c) 2025-2026 Gadec Software
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
 */

#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <configManager.hpp>

/**
 * @brief Manages the registration and execution of web handlers for the modern portal.
 */
class WebHandlerManager {
public:
    /**
     * @brief Constructor for WebHandlerManager.
     * @param server Reference to the active WebServer.
     * @param config Reference to the ConfigManager.
     */
    WebHandlerManager(WebServer& server, ConfigManager& config);

    /**
     * @brief Register all portal routes with the server.
     */
    void begin();

private:
    WebServer& _server;
    ConfigManager& _config;

    // --- Route Handlers ---
    
    /**
     * @brief Serve the modern portal index.html (gzipped).
     */
    void handlePortalRoot();

    /**
     * @brief Get system health and status as JSON.
     */
    void handleGetStatus();

    /**
     * @brief Save global configuration settings (legacy, to be removed)
     */
    void handleSaveConfig();

    /**
     * @brief Get unified configuration (System + Keys + Boards).
     */
    void handleGetConfig();

    /**
     * @brief Unified "Save & Apply" action.
     */
    void handleSaveAll();

    /**
     * @brief API: Scan for local WiFi networks.
     */
    void handleWiFiScan();

    /**
     * @brief API: Test a specific WiFi connection asynchronously.
     */
    void handleWiFiTest();

    /**
     * @brief API: Reset WiFi credentials and reboot into AP mode.
     */
    void handleWiFiReset();

    /**
     * @brief Get all boards configuration.
     */
    void handleGetBoards();

    /**
     * @brief CRUD: Add or update a board.
     */
    void handleSaveBoard();

    /**
     * @brief CRUD: Delete a board.
     */
    void handleDeepDeleteBoard();

    /**
     * @brief Get API Key Registry.
     */
    void handleGetKeys();

    /**
     * @brief CRUD: Save an API key.
     */
    void handleSaveKey();

    /**
     * @brief CRUD: Delete an API key.
     */
    void handleDeleteKey();

    /**
     * @brief API: Test an API key connection.
     */
    void handleTestKey();

    /**
     * @brief Captive portal redirect: Catch-all for unknown requests to redirect to /portal.
     */
    void handleCaptivePortalRedirect();

    // --- Helpers ---
    void sendGzipFlash(const uint8_t* data, size_t len, const char* contentType);
};
