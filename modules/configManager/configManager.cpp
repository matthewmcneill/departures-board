/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/configManager/configManager.cpp
 * Description: Implementation of configuration lifecycle methods including LittleFS 
 *              persistence and JSON serialization/deserialization.
 *
 * Exported Functions/Classes:
 * - ConfigManager::loadConfig: Load settings from /config.json.
 * - ConfigManager::loadApiKeys: Load secrets from /apikeys.json.
 * - ConfigManager::writeDefaultConfig: Generate skeleton configuration.
 * - ConfigManager::saveFile: LittleFS write wrapper.
 * - ConfigManager::loadFile: LittleFS read wrapper.
 */

#include "configManager.hpp"
#include <Logger.hpp>
#include <timeManager.hpp>

// Forward declaration of the configuration subscriber interface
#include "iConfigurable.hpp"

/**
 * @brief Utility to save string data to a file in LittleFS.
 * @param fName Absolute path to the destination file.
 * @param fData Content string to write.
 * @return True if write was successful.
 */
bool ConfigManager::saveFile(String fName, String fData) {
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
 * @brief Utility to load text from a file into a String object.
 * @param fName Absolute path to the source file.
 * @return String containing file contents, or empty on failure.
 */
String ConfigManager::loadFile(String fName) {
  File f = LittleFS.open(fName,"r");
  if (f) {
    String result = f.readString();
    f.close();
    return result;
  } else return "";
}


/**
 * @brief Load sensitive tokens (NR token, OWM token, TfL App Key) from `/apikeys.json`.
 */
void ConfigManager::loadApiKeys() {
  LOG_INFO("Loading API keys from /apikeys.json...");
  // --- Step 1: Open and Parse File ---
  JsonDocument doc; // JSON staging document

  if (LittleFS.exists(F("/apikeys.json"))) {
    File file = LittleFS.open(F("/apikeys.json"), "r");
    if (file) {
      DeserializationError error = deserializeJson(doc, file);
      if (!error) {
        JsonObject settings = doc.as<JsonObject>();

        if (settings[F("nrToken")].is<const char*>()) {
          strlcpy(config.nrToken, settings[F("nrToken")], sizeof(config.nrToken));
        }

        if (settings[F("appKey")].is<const char*>()) {
          strlcpy(config.tflAppkey, settings[F("appKey")], sizeof(config.tflAppkey));
        }
        config.apiKeysLoaded = true;

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
 * @brief Write a default config file
 */
void ConfigManager::writeDefaultConfig() {
    String defaultConfig = "{\"crs\":\"\",\"station\":\"\",\"lat\":0,\"lon\":0,\"weather\":" + String((config.owmToken[0])?"true":"false") + F(",\"sleep\":false,\"clock\":true,\"showDate\":false,\"showBus\":false,\"update\":false,\"sleepStarts\":23,\"sleepEnds\":8,\"brightness\":20,\"tubeId\":\"\",\"tubeName\":\"\",\"mode\":") + String((!config.nrToken[0])?"1":"0") + "}";
    saveFile(F("/config.json"),defaultConfig);
    config.crsCode[0] = '\0';
    config.tubeId[0] = '\0';
}

/**
 * @brief Load all user preferences and module settings from `/config.json`.
 */
void ConfigManager::loadConfig() {
  LOG_INFO("Loading configuration from /config.json...");
  // --- Step 1: Initialize System Defaults ---
  JsonDocument doc; // JSON staging document

  // Set defaults
  strlcpy(config.hostname, "DeparturesBoard", sizeof(config.hostname));
  config.crsCode[0] = '\0';
  strlcpy(config.timezone, TimeManager::ukTimezone, sizeof(config.timezone));
  timeManager.setTimezone(String(config.timezone));

  if (LittleFS.exists(F("/config.json"))) {
    File file = LittleFS.open(F("/config.json"), "r");
    if (file) {
      DeserializationError error = deserializeJson(doc, file);
      if (!error) {
        JsonObject settings = doc.as<JsonObject>();

        if (settings[F("noScroll")].is<bool>())          config.noScrolling = settings[F("noScroll")];
        if (settings[F("flip")].is<bool>())              config.flipScreen = settings[F("flip")];
        if (settings[F("brightness")].is<int>())         config.brightness = settings[F("brightness")];
        if (settings[F("sleep")].is<bool>())             config.sleepEnabled = settings[F("sleep")];
        if (settings[F("sleepStarts")].is<int>())        config.sleepStarts = settings[F("sleepStarts")];
        if (settings[F("sleepEnds")].is<int>())          config.sleepEnds = settings[F("sleepEnds")];
        if (settings[F("TZ")].is<const char*>())         strlcpy(config.timezone, settings[F("TZ")], sizeof(config.timezone));
        timeManager.setTimezone(String(config.timezone));

        if (settings[F("mode")].is<int>())               config.boardMode = (BoardModes)settings[F("mode")];
        if (settings[F("showDate")].is<bool>())          config.dateEnabled = settings[F("showDate")];
        if (settings[F("clock")].is<bool>())             config.showClockInSleep = settings[F("clock")];
        if (settings[F("fastRefresh")].is<bool>())       config.apiRefreshRate = settings[F("fastRefresh")] ? FASTDATAUPDATEINTERVAL : DATAUPDATEINTERVAL;
        if (settings[F("update")].is<bool>())            config.firmwareUpdatesEnabled = settings[F("update")];
        if (settings[F("updateDaily")].is<bool>())       config.dailyUpdateCheckEnabled = settings[F("updateDaily")];

        // National Rail
        if (settings[F("crs")].is<const char*>())        strlcpy(config.crsCode, settings[F("crs")], sizeof(config.crsCode));
        if (settings[F("lat")].is<float>())              config.stationLat = settings[F("lat")];
        if (settings[F("lon")].is<float>())              config.stationLon = settings[F("lon")];
        if (settings[F("callingCrs")].is<const char*>()) strlcpy(config.callingCrsCode, settings[F("callingCrs")], sizeof(config.callingCrsCode));
        if (settings[F("callingStation")].is<const char*>()) strlcpy(config.callingStation, settings[F("callingStation")], sizeof(config.callingStation));
        if (settings[F("platformFilter")].is<const char*>()) strlcpy(config.platformFilter, settings[F("platformFilter")], sizeof(config.platformFilter));
        if (settings[F("nrTimeOffset")].is<int>())       config.nrTimeOffset = settings[F("nrTimeOffset")];

        // Alternative Rail
        if (settings[F("altCrs")].is<const char*>())     strlcpy(config.altCrsCode, settings[F("altCrs")], sizeof(config.altCrsCode));
        config.altStationEnabled = (config.altCrsCode[0] != '\0');
        if (settings[F("altStarts")].is<int>())          config.altStarts = settings[F("altStarts")];
        if (settings[F("altEnds")].is<int>())            config.altEnds = settings[F("altEnds")];
        if (settings[F("altLat")].is<float>())           config.altLat = settings[F("altLat")];
        if (settings[F("altLon")].is<float>())           config.altLon = settings[F("altLon")];
        if (settings[F("altCallingCrs")].is<const char*>()) strlcpy(config.altCallingCrsCode, settings[F("altCallingCrs")], sizeof(config.altCallingCrsCode));
        if (settings[F("altCallingStation")].is<const char*>()) strlcpy(config.altCallingStation, settings[F("altCallingStation")], sizeof(config.altCallingStation));
        if (settings[F("altPlatformFilter")].is<const char*>()) strlcpy(config.altPlatformFilter, settings[F("altPlatformFilter")], sizeof(config.altPlatformFilter));

        // TfL
        if (settings[F("tubeId")].is<const char*>())     strlcpy(config.tubeId, settings[F("tubeId")], sizeof(config.tubeId));
        if (settings[F("tubeName")].is<const char*>()) {
            String tName = settings[F("tubeName")].as<String>();
            if (tName.endsWith(F(" Underground Station"))) tName.remove(tName.length()-20);
            else if (tName.endsWith(F(" DLR Station"))) tName.remove(tName.length()-12);
            else if (tName.endsWith(F(" (H&C Line)"))) tName.remove(tName.length()-11);
            strlcpy(config.tubeName, tName.c_str(), sizeof(config.tubeName));
        }

        // Bus
        if (settings[F("showBus")].is<bool>())           config.showBus = settings[F("showBus")];
        if (settings[F("busId")].is<const char*>())      strlcpy(config.busId, settings[F("busId")], sizeof(config.busId));
        if (settings[F("busName")].is<const char*>())    strlcpy(config.busName, settings[F("busName")], sizeof(config.busName));
        if (settings[F("busLat")].is<float>())           config.busLat = settings[F("busLat")];
        if (settings[F("busLon")].is<float>())           config.busLon = settings[F("busLon")];
        if (settings[F("busFilter")].is<const char*>())  strlcpy(config.busFilter, settings[F("busFilter")], sizeof(config.busFilter));

        // RSS
        if (settings[F("rssUrl")].is<const char*>())     strlcpy(config.rssUrl, settings[F("rssUrl")], sizeof(config.rssUrl));
        if (settings[F("rssName")].is<const char*>())    strlcpy(config.rssName, settings[F("rssName")], sizeof(config.rssName));
        config.rssEnabled = (config.rssUrl[0] != '\0');

      } else {
        LOG_ERROR(String("Failed to parse /config.json: ") + error.c_str());
      }
      file.close();
    } else {
      LOG_ERROR("Failed to open /config.json for reading.");
    }
  } else {
    LOG_INFO("/config.json not found. Creating default config.");
    if (config.nrToken[0] || config.tflAppkey[0]) writeDefaultConfig();
  }
}

/**
 * @brief Register a component to receive configuration updates.
 * @param consumer Pointer to the component to register.
 */
void ConfigManager::registerConsumer(iConfigurable* consumer) {
    if (consumerCount < MAX_CONSUMERS) {
        consumers[consumerCount++] = consumer;
    }
}

/**
 * @brief Notify all registered consumers to re-apply the current configuration.
 */
void ConfigManager::notifyConsumersToReapplyConfig() {
    for (int i = 0; i < consumerCount; i++) {
        if (consumers[i]) {
            consumers[i]->reapplyConfig(config);
        }
    }
}
