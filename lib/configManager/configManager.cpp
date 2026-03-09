/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/configManager/configManager.cpp
 * Description: Implementation of configuration lifecycle methods, handling
 *              JSON loading and delegation of properties to specific boards or clients.
 */

#include "configManager.hpp"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <githubClient.h>
#include <JsonListener.h>
#include <JsonStreamingParser.h>
#include <timeManager.hpp>
#include <Logger.hpp>
#include <displayManager.hpp>
#include "../boards/nationalRailBoard/include/nationalRailBoard.hpp"
#include <otaUpdater.hpp>
#include <drawingPrimitives.hpp>
#include "../../include/webgui/index.h"
#include <rssClient.h>
#include <weatherClient.h>

extern weatherClient* currentWeather;
extern rssClient* rss;

/**
 * @brief Utility to save string data to a file in LittleFS
 * @param fName Path to the file
 * @param fData Content to save
 * @return true if successful
 */
bool saveFile(String fName, String fData) {
  LOG_INFO(String("Attempting to save ") + fData.length() + " bytes to file: " + fName);
  File f = LittleFS.open(fName,"w");
  if (f) {
    f.println(fData);
    f.close();
    LOG_INFO(String("Successfully saved file: ") + fName);
    return true;
  } else {
    LOG_ERROR(String("Failed to open file for writing: ") + fName);
    char fsErr[64];
    sprintf(fsErr, "LittleFS Storage: %zu total, %zu used.", LittleFS.totalBytes(), LittleFS.usedBytes());
    LOG_ERROR(String(fsErr));
    return false;
  }
}

/**
 * @brief Utility to load text from a file into a String object
 * @param fName Path to the file
 * @return File contents or empty string if failed
 */
String loadFile(String fName) {
  File f = LittleFS.open(fName,"r");
  if (f) {
    String result = f.readString();
    f.close();
    return result;
  } else return "";
}

/**
 * @brief Check post web upgrade
 */
void checkPostWebUpgrade() {
  String prevGUI = loadFile(F("/webver"));
  prevGUI.trim();
  String currentGUI = String(WEBAPPVER_MAJOR) + F(".") + String(WEBAPPVER_MINOR);
  if (prevGUI != currentGUI) {
    // clean up old/dev files
    progressBar(F("Cleaning up following upgrade"),45);
    LittleFS.remove(F("/index_d.htm"));
    LittleFS.remove(F("/index.htm"));
    LittleFS.remove(F("/keys.htm"));
    LittleFS.remove(F("/nrelogo.webp"));
    LittleFS.remove(F("/tfllogo.webp"));
    LittleFS.remove(F("/btlogo.webp"));
    LittleFS.remove(F("/tube.webp"));
    LittleFS.remove(F("/nr.webp"));
    LittleFS.remove(F("/favicon.svg"));
    LittleFS.remove(F("/favicon.png"));
    saveFile(F("/webver"),currentGUI);
  }
}

/**
 * @brief Load keys from apikeys.json
 */
void loadApiKeys() {
  LOG_INFO("Loading API keys from /apikeys.json...");
  JsonDocument doc;

  if (LittleFS.exists(F("/apikeys.json"))) {
    File file = LittleFS.open(F("/apikeys.json"), "r");
    if (file) {
      DeserializationError error = deserializeJson(doc, file);
      if (!error) {
        JsonObject settings = doc.as<JsonObject>();

        if (settings[F("nrToken")].is<const char*>()) {
          nationalRailBoard.setNrToken(settings[F("nrToken")]);
        }

        if (settings[F("owmToken")].is<const char*>()) {
          currentWeather->setOpenWeatherMapApiKey(settings[F("owmToken")]);
        }

        if (settings[F("appKey")].is<const char*>()) {
          tflBoard->setTflAppkey(settings[F("appKey")]);
        }
        apiKeys = true;

      } else {
        LOG_ERROR(String("Failed to parse /apikeys.json: ") + error.c_str());
      }
      file.close();
    } else {
      LOG_ERROR("Failed to open /apikeys.json for reading.");
    }
  } else {
    LOG_INFO("/apikeys.json not found on LittleFS.");
  }
}

/**
 * @brief Write a default config file so that the Web GUI works initially (forces Tube mode if no NR token).
 */
void writeDefaultConfig() {
    String defaultConfig = "{\"crs\":\"\",\"station\":\"\",\"lat\":0,\"lon\":0,\"weather\":" + String((currentWeather->getOpenWeatherMapApiKey()[0])?"true":"false") + F(",\"sleep\":false,\"showDate\":false,\"showBus\":false,\"update\":false,\"sleepStarts\":23,\"sleepEnds\":8,\"brightness\":20,\"tubeId\":\"\",\"tubeName\":\"\",\"mode\":") + String((!nationalRailBoard.getNrToken()[0])?"1":"0") + "}";
    saveFile(F("/config.json"),defaultConfig);
    nationalRailBoard.setCrsCode("");
    tflBoard->setTubeId("");
}

/**
 * @brief Load settings from config.json and populate global variables
 */
void loadConfig() {
  LOG_INFO("Loading configuration from /config.json...");
  JsonDocument doc;

  // Set defaults
  strcpy(hostname,defaultHostname);
  nationalRailBoard.setCrsCode("");
  timeManager.setTimezone(String(TimeManager::ukTimezone));

  if (LittleFS.exists(F("/config.json"))) {
    File file = LittleFS.open(F("/config.json"), "r");
    if (file) {
      DeserializationError error = deserializeJson(doc, file);
      if (!error) {
        JsonObject settings = doc.as<JsonObject>();

        if (settings[F("crs")].is<const char*>())        nationalRailBoard.setCrsCode(settings[F("crs")]);
        if (settings[F("callingCrs")].is<const char*>()) nationalRailBoard.setCallingCrsCode(settings[F("callingCrs")]);
        if (settings[F("callingStation")].is<const char*>()) nationalRailBoard.setCallingStation(settings[F("callingStation")]);
        if (settings[F("platformFilter")].is<const char*>())  nationalRailBoard.setPlatformFilter(settings[F("platformFilter")]);
        if (settings[F("hostname")].is<const char*>())   strlcpy(hostname, settings[F("hostname")], sizeof(hostname));
        if (settings[F("wsdlHost")].is<const char*>())   strlcpy(wsdlHost, settings[F("wsdlHost")], sizeof(wsdlHost));
        if (settings[F("wsdlAPI")].is<const char*>())    strlcpy(wsdlAPI, settings[F("wsdlAPI")], sizeof(wsdlAPI));
        if (settings[F("showDate")].is<bool>())          dateEnabled = settings[F("showDate")];
        if (settings[F("showBus")].is<bool>())           busBoard->setEnableBus(settings[F("showBus")]);
        if (settings[F("sleep")].is<bool>())             displayManager.setSleepEnabled(settings[F("sleep")]);
        if (settings[F("fastRefresh")].is<bool>())       apiRefreshRate = settings[F("fastRefresh")] ? FASTDATAUPDATEINTERVAL : DATAUPDATEINTERVAL;
        if (settings[F("weather")].is<bool>() && currentWeather->getOpenWeatherMapApiKey()[0])
                                                    currentWeather->setWeatherEnabled(settings[F("weather")]);
        if (settings[F("update")].is<bool>())            ota.setFirmwareUpdatesEnabled(settings[F("update")]);
        if (settings[F("updateDaily")].is<bool>())       ota.setDailyUpdateCheckEnabled(settings[F("updateDaily")]);
        if (settings[F("sleepStarts")].is<int>())        displayManager.setSleepStarts(settings[F("sleepStarts")]);
        if (settings[F("sleepEnds")].is<int>())          displayManager.setSleepEnds(settings[F("sleepEnds")]);
        if (settings[F("brightness")].is<int>())         displayManager.setBrightness(settings[F("brightness")]);
        if (settings[F("lat")].is<float>())              nationalRailBoard.setStationLat(settings[F("lat")]);
        if (settings[F("lon")].is<float>())              nationalRailBoard.setStationLon(settings[F("lon")]);

        if (settings[F("mode")].is<int>())               boardMode = settings[F("mode")];
        else if (settings[F("tube")].is<bool>())         boardMode = settings[F("tube")] ? MODE_TUBE : MODE_RAIL; // handle legacy v1.x config
        if (settings[F("tubeId")].is<const char*>())     tflBoard->setTubeId(settings[F("tubeId")]);
        if (settings[F("tubeName")].is<const char*>())   tflBoard->setTubeName(settings[F("tubeName")]);

        String tName = String(tflBoard->getTubeName());
        // Clean up the underground station name
        if (tName.endsWith(F(" Underground Station"))) tName.remove(tName.length()-20);
        else if (tName.endsWith(F(" DLR Station"))) tName.remove(tName.length()-12);
        else if (tName.endsWith(F(" (H&C Line)"))) tName.remove(tName.length()-11);
        tflBoard->setTubeName(tName.c_str());

        if (settings[F("altCrs")].is<const char*>())     nationalRailBoard.setAltCrsCode(settings[F("altCrs")]);
        if (nationalRailBoard.getAltCrsCode()[0]) nationalRailBoard.setAltStationEnabled(true); else nationalRailBoard.setAltStationEnabled(false);
        if (settings[F("altStarts")].is<int>())          nationalRailBoard.setAltStarts(settings[F("altStarts")]);
        if (settings[F("altEnds")].is<int>())            nationalRailBoard.setAltEnds(settings[F("altEnds")]);
        if (settings[F("altLat")].is<float>())           nationalRailBoard.setAltLat(settings[F("altLat")]);
        if (settings[F("altLon")].is<float>())           nationalRailBoard.setAltLon(settings[F("altLon")]);
        if (settings[F("altCallingCrs")].is<const char*>()) nationalRailBoard.setAltCallingCrsCode(settings[F("altCallingCrs")]);
        if (settings[F("altCallingStation")].is<const char*>()) nationalRailBoard.setAltCallingStation(settings[F("altCallingStation")]);
        if (settings[F("altPlatformFilter")].is<const char*>())  nationalRailBoard.setAltPlatformFilter(settings[F("altPlatformFilter")]);

        if (settings[F("busId")].is<const char*>())      busBoard->setBusAtco(settings[F("busId")]);
        if (settings[F("busName")].is<const char*>())    busBoard->setBusName(settings[F("busName")]);
        if (settings[F("busLat")].is<float>())           busBoard->setBusLat(settings[F("busLat")]);
        if (settings[F("busLon")].is<float>())           busBoard->setBusLon(settings[F("busLon")]);
        if (settings[F("busFilter")].is<const char*>())  busBoard->setBusFilter(settings[F("busFilter")]);

        if (settings[F("noScroll")].is<bool>())          noScrolling = settings[F("noScroll")];
        if (settings[F("flip")].is<bool>())              displayManager.setFlipScreen(settings[F("flip")]);
        if (settings[F("TZ")].is<const char*>())         timeManager.setTimezone(settings[F("TZ")].as<String>());
        if (settings[F("nrTimeOffset")].is<int>())       nationalRailBoard.setNrTimeOffset(settings[F("nrTimeOffset")]);
        if (settings[F("hidePlatform")].is<bool>())      hidePlatform = settings[F("hidePlatform")];

        if (settings[F("rssUrl")].is<const char*>())     rss->setRssURL(settings[F("rssUrl")]);
        if (settings[F("rssName")].is<const char*>())    rss->setRssName(settings[F("rssName")]);
        if (rss->getRssURL()[0]) rss->setRssEnabled(true); else rss->setRssEnabled(false);

      } else {
        LOG_ERROR(String("Failed to parse /config.json: ") + error.c_str());
      }
      file.close();
    } else {
      LOG_ERROR("Failed to open /config.json for reading.");
    }
  } else {
    LOG_INFO("/config.json not found. Creating default config.");
    if (nationalRailBoard.getNrToken()[0] || tflBoard->getTflAppkey()[0]) writeDefaultConfig();
  }
}
