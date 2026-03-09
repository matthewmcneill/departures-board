/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/webServer/webServer.cpp
 * Description: Defines web endpoints and file manipulation routines for LittleFS.
 */

#include "webServer.hpp"
#include <Update.h>
#include <LittleFS.h>
#include <WiFi.h>
#include <Logger.hpp>
#include <displayManager.hpp>
#include <configManager.hpp>
#include "../boards/systemBoard/include/systemBoard.hpp"
#include "../boards/nationalRailBoard/include/nationalRailBoard.hpp"
#include <weatherClient.h>
#include <rssClient.h>
#include <WiFiClientSecure.h>
#include "../../include/webgui/index.h"
#include "../../include/webgui/pages.h"
#include "../../include/webgui/webgraphics.h"
#include "../../include/webgui/keys.h"
#include "../../include/webgui/rssfeeds.h"
#include <githubClient.h>

extern weatherClient* currentWeather;
extern rssClient* rss;
extern stnMessages messages;
extern github ghUpdate;
extern int dataLoadSuccess;
extern int dataLoadFailure;
extern unsigned long lastDataLoadTime;
extern NationalRailBoard nationalRailBoard;
extern unsigned long lastLoadFailure;
extern int lastUpdateResult;
extern int lastRssUpdateResult;
extern bool forcedSleep;
extern bool sleepClock;
extern struct tm timeinfo;

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

void WebServerManager::init() {
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
  server.on(F("/stationpicker"),handleStationPicker);           // Used by the Web GUI to lookup station codes interactively
  server.on(F("/firmware"),handleFirmwareInfo);                 // Used by the Web GUI to display the running firmware version
  server.on(F("/savesettings"),HTTP_POST,handleSaveSettings);   // Used by the Web GUI to save updated configuration settings
  server.on(F("/savekeys"),HTTP_POST,handleSaveKeys);           // Used by the Web GUI to verify/save API keys
  server.on(F("/brightness"),handleBrightness);                 // Used by the Web GUI to interactively set the panel brightness
  server.on(F("/ota"),handleOtaUpdate);                         // Used by the Web GUI to initiate a manual firmware/WebApp update
  server.on(F("/control"),handleControl);                       // Endpoint for automation
  
  LOG_INFO("Local webserver endpoints configured.");

  server.on(F("/update"), HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, contentTypeHtml, updatePage);
  });
  /*handling uploading firmware file */
  server.on(F("/update"), HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    sendResponse(200,(Update.hasError()) ? "FAIL" : "OK");
    delay(500);
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        //Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        //Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
      } else {
        //Update.printError(Serial);
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

  server.begin();     // Start the local web server
  LOG_INFO("Local webserver started on port 80.");
}

void WebServerManager::handleClient() {
    server.handleClient();
}
