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
 */

#pragma once
#include <memory>

class AsyncWebServer;
class WebHandlerManager;

class WebServerManager {
public:
    WebServerManager();
    ~WebServerManager();
    
    /**
     * @brief Setup the physical HTTP routing endpoints and firmware upload callbacks.
     */
    void init();

    /**
     * @brief Proxy to correctly format and trigger a weather update via the weatherClient.
     */
    void updateCurrentWeather(float lat, float lon);

    AsyncWebServer& getServer() { return *_server; }

private:
    std::unique_ptr<AsyncWebServer> _server;
    std::unique_ptr<WebHandlerManager> _handlerManager;
};
