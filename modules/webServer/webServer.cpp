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
#include <wifiManager.hpp>
#include "webHandlerManager.hpp"
#include <SPIFFS.h>
#include <Update.h>
#include <LittleFS.h>
#include <WiFi.h>
#include <logger.hpp>
#include <displayManager.hpp>
#include <configManager.hpp>
#include <boards/systemBoard/firmwareUpdateBoard.hpp>
#include <boards/nationalRailBoard/nationalRailBoard.hpp>
#include <weatherClient.hpp>
#include <rssClient.hpp>
#include <WiFiClientSecure.h>
#include <appContext.hpp>
#include <githubClient.hpp>
#include <otaUpdater.hpp>


extern class appContext appContext;

// Instantiate the global server and manager
WebServer server(80);
WebServerManager webServer;
File fsUploadFile;


/**
 * @brief Initializes the web server and binds all application endpoints.
 */
void WebServerManager::init() {
  server.on(F("/"), HTTP_GET, []() {
      server.sendHeader("Location", "/web", true);
      server.send(302, "text/plain", "");
  });
 


 
  // Initialize and register new portal handlers
  _handlerManager = new WebHandlerManager(server, appContext.getConfigManager());
  _handlerManager->begin();

  server.begin();     
  LOG_INFO("WEB", "Local webserver started at http://" + WiFi.localIP().toString() + ":80/");
}

void WebServerManager::handleClient() {
    if (isHandlingClient) return;
    isHandlingClient = true;
    server.handleClient();
    isHandlingClient = false;
    yield();
}
