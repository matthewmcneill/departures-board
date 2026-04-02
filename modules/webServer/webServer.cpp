/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * Module: modules/webServer/webServer.cpp
 * Description: Orchestrates the local HTTP server, endpoint routing, and
 *              LittleFS file management routines for the Web GUI.
 */

#include "webServer.hpp"
#include "webHandlerManager.hpp"
#include <LittleFS.h>
#include <Update.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <appContext.hpp>
#include <boards/nationalRailBoard/nationalRailBoard.hpp>
#include <boards/systemBoard/firmwareUpdateBoard.hpp>
#include <configManager.hpp>
#include <displayManager.hpp>
#include <githubClient.hpp>
#include <logger.hpp>
#include <otaUpdater.hpp>
#include <rssClient.hpp>
#include <weatherClient.hpp>
#include <wifiManager.hpp>

extern class appContext appContext;

// Instantiate the global server and manager
AsyncWebServer server(80);
WebServerManager webServer;
File fsUploadFile;

WebServerManager::WebServerManager() : _handlerManager(nullptr) {
}

WebServerManager::~WebServerManager() {
    // std::unique_ptr automatically handles deallocation of _handlerManager
}

/**
 * @brief Initializes the web server and binds all application endpoints.
 */
void WebServerManager::init() {
  server.on("/", HTTP_GET,
            [](AsyncWebServerRequest *request) { request->redirect("/web"); });

  // Initialize and register new portal handlers
  _handlerManager =
      std::make_unique<WebHandlerManager>(server, appContext.getConfigManager());
  _handlerManager->begin();

  server.begin();
  LOG_INFO("WEB", "Local async webserver started at http://" +
                      WiFi.localIP().toString() + ":80/");
}
