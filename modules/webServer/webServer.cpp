/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/webServer/webServer.cpp
 * Description: Orchestrates the local HTTP server, endpoint routing, and 
 *              LittleFS file management routines for the Web GUI.
 *
 * Exported Functions/Classes:
 * - WebServerManager::init: Configures all REST and static file endpoints.
 * - WebServerManager::handleClient: Yields process to the HTTP stack.
 */

#include "webServer.hpp"
#include <WiFiManager.h>
#include <SPIFFS.h>
#include <Update.h>
#include <LittleFS.h>
#include <WiFi.h>
#include <Logger.hpp>
#include <displayManager.hpp>
#include <configManager.hpp>
#include <boards/systemBoard/systemBoard.hpp>
#include <boards/systemBoard/firmwareUpdateBoard.hpp>
#include <boards/nationalRailBoard/nationalRailBoard.hpp>
#include <weatherClient.h>
#include <rssClient.h>
#include <WiFiClientSecure.h>
#include "../../include/webgui/index.h"
#include "../../include/webgui/pages.h"
#include "../../include/webgui/rssfeeds.h"
#include "../../include/webgui/webgraphics.h"
#include "../../include/webgui/keys.h"
#include <githubClient.h>

extern weatherClient* currentWeather;    // Weather service for verification
extern rssClient* rss;                    // RSS headlines handler
extern stnMessages messages;          // Primary message string pool
extern github ghUpdate;                   // GitHub API client for OTA
extern int dataLoadSuccess;               // Cumulative success counter
extern int dataLoadFailure;               // Cumulative failure counter
extern unsigned long lastDataLoadTime;    // Timestamp (ms) of last fetch
extern NationalRailBoard nationalRailBoard; // Primary board implementation
extern unsigned long lastLoadFailure;     // Timestamp (ms) of last exception
extern int lastUpdateResult;              // Last HTTP status code
extern int lastRssUpdateResult;           // Last RSS status code
extern ConfigManager configManager;       // Central configuration store
extern struct tm timeinfo;                // Sychronized system time

#include <otaUpdater.hpp>

#define msDay 86400000
#define msHour 3600000
#define msMin 60000

// Shorthand for response formats
static const char contentTypeJson[] PROGMEM = "application/json";
static const char contentTypeText[] PROGMEM = "text/plain";
static const char contentTypeHtml[] PROGMEM = "text/html";

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
  // --- Step 1: Bind System and Configuration Endpoints ---
  server.on(F("/"),handleRoot);
  server.on(F("/erasewifi"),handleEraseWiFi);
  server.on(F("/factoryreset"),handleFactoryReset);
  server.on(F("/info"),handleInfo);
  server.on(F("/formatffs"),handleFormatFFS);
  server.on(F("/dir"),handleFileList);
  server.onNotFound(handleNotFound);
  server.on(F("/cat"),handleCat);
  server.on(F("/del"),handleDelete);
  server.on(F("/reboot"),handleReboot);
 
  // --- Step 2: Bind UI and Interaction Endpoints ---
  server.on(F("/stationpicker"),handleStationPicker);           
  server.on(F("/firmware"),handleFirmwareInfo);                 
  server.on(F("/savesettings"),HTTP_POST,handleSaveSettings);   
  server.on(F("/savekeys"),HTTP_POST,handleSaveKeys);           
  server.on(F("/brightness"),handleBrightness);                 
  server.on(F("/ota"),handleOtaUpdate);                         
  server.on(F("/control"),handleControl);                       
  
  LOG_INFO("Local webserver endpoints configured.");
 
  // --- Step 3: Bind Update and Upload Handlers ---
  server.on(F("/update"), HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, contentTypeHtml, updatePage);
  });
  
  /* Binary Firmware Flash Endpoint */
  server.on(F("/update"), HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    sendResponse(200,(Update.hasError()) ? "FAIL" : "OK");
    delay(500);
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { 
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (!Update.end(true)) { 
      }
    }
  });
 
  server.on(F("/upload"), HTTP_GET, []() {
      server.send(200, contentTypeHtml, uploadPage);
  });
  server.on(F("/upload"), HTTP_POST, []() {
  }, handleFileUpload);
 
  server.on(F("/success"), HTTP_GET, []() {
    server.send(200, contentTypeHtml, successPage);
  });
 
  // --- Step 4: Finalise Activation ---
  server.begin();     
  char startMsg[64];
  snprintf(startMsg, sizeof(startMsg), "Local webserver started at http://%s", WiFi.localIP().toString().c_str());
  LOG_INFO(startMsg);
}

void WebServerManager::handleClient() {
    server.handleClient();
}
