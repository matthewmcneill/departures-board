/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * Module: modules/webServer/webServer.cpp
 * Description: Orchestrates the local HTTP server, endpoint routing, and
 *              LittleFS file management routines for the Web GUI.
 *
 * Exported Functions/Classes:
 * - WebServerManager: [Class implementation]
 *   - init: Master endpoint configuration and server start.
 *   - updateCurrentWeather: Triggered logic for manual geo-sync.
 */

#include "webServer.hpp"
#include "webHandlerManager.hpp"
#include <Arduino.h>
#include <LittleFS.h>
#include <WiFi.h>
#include <appContext.hpp>
#include <weatherClient.hpp>
#include <displayManager.hpp>
#include <configManager.hpp>
#include <boards/interfaces/iDisplayBoard.hpp>
#include <logger.hpp>

extern class appContext appContext;

// File handle for firmware uploads
File fsUploadFile;

/**
 * @brief Construct a new WebServerManager.
 * Statically allocates the AsyncWebServer on port 80.
 */
WebServerManager::WebServerManager() 
    : _server(std::make_unique<AsyncWebServer>(80)), _handlerManager(nullptr) {
}

/**
 * @brief Destructor. 
 */
WebServerManager::~WebServerManager() {
    // std::unique_ptr automatically handles deallocation of _handlerManager
}

/**
 * @brief Initializes the web server and binds all application endpoints.
 */
void WebServerManager::init() {
  _server->on("/", HTTP_GET,
            [](AsyncWebServerRequest *request) { request->redirect("/web"); });

  // Initialize and register new portal handlers
  _handlerManager =
      std::make_unique<WebHandlerManager>(*_server, appContext);
  _handlerManager->begin();

  _server->begin();
  LOG_INFO("WEB", "Local async webserver started at http://" +
                      WiFi.localIP().toString() + ":80/");
}

/**
 * @brief Proxy a weather request from the web interface to the weatherClient.
 * @param lat Decimal latitude.
 * @param lon Decimal longitude.
 */
void WebServerManager::updateCurrentWeather(float lat, float lon) {
    weatherClient& weather = appContext.getWeather();
    if (weather.getWeatherEnabled()) {
        int activeIndex = appContext.getDisplayManager().getActiveSlotIndex();
        iDisplayBoard* active = appContext.getDisplayManager().getDisplayBoard(activeIndex);
        if (active) {
            WeatherStatus& ws = active->getWeatherStatus();
            ws.lat = lat;
            ws.lon = lon;
            const Config& config = appContext.getConfigManager().getConfig();
            weather.updateWeather(ws, config.weatherKeyId);
        }
    }
}
