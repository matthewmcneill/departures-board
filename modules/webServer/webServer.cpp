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
#include "../../include/webgui/index.h"
#include "../../include/webgui/pages.h"
#include "../../include/webgui/rssfeeds.h"
#include "../../include/webgui/webgraphics.h"
#include "../../include/webgui/keys.h"

extern class appContext appContext;

// Instantiate the global server and manager
WebServer server(80);
WebServerManager webServer;
File fsUploadFile;

// Include the handlers directly since they rely on global variables and `server`
#include "../../include/webgui/WebHandlers.hpp"

/**
 * @brief Initializes the web server and binds all application endpoints.
 */
void WebServerManager::init() {
  server.on(F("/"), handleRoot);
  server.on(F("/erasewifi"), handleEraseWiFi);
  server.on(F("/factoryreset"), handleFactoryReset);
  server.on(F("/info"), handleInfo);
  server.on(F("/formatffs"), handleFormatFFS);
  server.on(F("/dir"), handleFileList);
  server.onNotFound(handleNotFound);
  server.on(F("/cat"), handleCat);
  server.on(F("/del"), handleDelete);
  server.on(F("/reboot"), handleReboot);
 
  server.on(F("/stationpicker"), handleStationPicker);           
  server.on(F("/firmware"), handleFirmwareInfo);                 
  server.on(F("/config.json"), handleGetConfigSettings);         
  server.on(F("/savesettings"), HTTP_POST, handleSaveSettings);   
  server.on(F("/savekeys"), HTTP_POST, handleSaveKeys);           
  server.on(F("/brightness"), handleBrightness);                 
  server.on(F("/ota"), handleOtaUpdate);                         
  server.on(F("/control"), handleControl);                       
  
  LOG_INFO("WEB", "Local webserver endpoints configured.");
 
  server.on(F("/update"), HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", updatePage);
  });
  
  server.on(F("/update"), HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    sendResponse(200, (Update.hasError()) ? F("FAIL") : F("OK"));
    delay(500);
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Update.begin(UPDATE_SIZE_UNKNOWN);
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      Update.write(upload.buf, upload.currentSize);
    } else if (upload.status == UPLOAD_FILE_END) {
      Update.end(true);
    }
  });
 
  server.on(F("/upload"), HTTP_GET, []() {
      server.send(200, "text/html", uploadPage);
  });
  server.on(F("/upload"), HTTP_POST, []() {}, handleFileUpload);
 
  server.on(F("/success"), HTTP_GET, []() {
    server.send(200, "text/html", successPage);
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
