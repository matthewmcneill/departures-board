/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/webServer/webServer.hpp
 * Description: Instantiates ESP32 HTTP WebServer endpoints and GUI handles.
 *
 * Provides:
 * - WebServerManager: Encapsulates web handlers, firmware uploads, and API endpoints. 
 * - webServer: Standard instantiation of WebServerManager.
 * - server: Underlying ESP32 WebServer backend.
 */

#pragma once

#include <WebServer.h>

class WebHandlerManager;

class WebServerManager {
public:
    /**
     * @brief Setup the physical HTTP routing endpoints and firmware upload callbacks.
     */
    void init();

    /**
     * @brief Hand control to the HTTP connection listener if any clients are waiting.
     */
    void handleClient();

private:
    WebHandlerManager* _handlerManager = nullptr;
};

extern WebServerManager webServer;
extern WebServer server;

void updateCurrentWeather(float latitude, float longitude);
