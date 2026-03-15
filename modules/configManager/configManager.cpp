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
#include <logger.hpp>
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
  LOG_INFO("CONFIG", String("Attempting to save ") + fData.length() + " bytes to file: " + fName);
  File f = LittleFS.open(fName, "w");
  if (f) {
    f.print(fData);
    f.close();
    LOG_INFO("CONFIG", String("Successfully saved file: ") + fName);
    return true;
  } else {
    LOG_ERROR("CONFIG", String("Failed to open file for writing: ") + fName);
    String fsErr = "ERR_LITTLEFS_WRITE";
    LOG_ERROR("CONFIG", String(fsErr));
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
  LOG_INFO("CONFIG", "Loading API keys from /apikeys.json...");
  // --- Step 1: Open and Parse File ---
  JsonDocument doc; // JSON staging document

  if (LittleFS.exists("/apikeys.json")) {
    String contents = loadFile("/apikeys.json");
    LOG_DEBUG("CONFIG", String("Dumping /apikeys.json:\n") + contents);
    
    DeserializationError error = deserializeJson(doc, contents);
    if (!error) {
        JsonObject settings = doc.as<JsonObject>();

        if (settings[F("nrToken")].is<const char*>()) {
          strlcpy(config.nrToken, settings[F("nrToken")], sizeof(config.nrToken));
        }

        if (settings[F("appKey")].is<const char*>()) {
          strlcpy(config.tflAppkey, settings[F("appKey")], sizeof(config.tflAppkey));
        }

        if (settings[F("owmToken")].is<const char*>()) {
          strlcpy(config.owmToken, settings[F("owmToken")], sizeof(config.owmToken));
        }

        config.apiKeysLoaded = true;

        // Register secrets for automatic log redaction
        if (config.nrToken[0]) Logger::registerSecret(config.nrToken);
        if (config.tflAppkey[0]) Logger::registerSecret(config.tflAppkey);
        if (config.owmToken[0]) Logger::registerSecret(config.owmToken);

        LOG_INFO("SYSTEM", "API keys loaded (NR=" + String(config.nrToken[0] ? "SET" : "MISSING") + 
                           ", TfL=" + String(config.tflAppkey[0] ? "SET" : "MISSING") + 
                           ", OWM=" + String(config.owmToken[0] ? "SET" : "MISSING") + ")");

      } else {
        LOG_ERROR("CONFIG", String("Failed to parse /apikeys.json: ") + error.c_str());
      }
  } else {
    LOG_WARN("CONFIG", "/apikeys.json not found on LittleFS.");
  }
}

/**
 * @brief Write a default configuration file with 4 standard boards.
 */
void ConfigManager::writeDefaultConfig() {
    LOG_INFO("CONFIG", "Generating default configuration...");
    JsonDocument doc;
    
    doc[F("hostname")] = "DeparturesBoard";
    doc[F("version")] = 2.1; // Current config format version
    doc[F("brightness")] = 20;
    doc[F("sleep")] = false;
    doc[F("clock")] = true;
    doc[F("showDate")] = false;
    doc[F("flip")] = false;
    doc[F("TZ")] = TimeManager::ukTimezone;
    doc[F("mode")] = config.defaultBoardIndex;

    JsonArray boards = doc[F("boards")].to<JsonArray>();

    // 1. National Rail (Main)
    JsonObject br1 = boards.add<JsonObject>();
    br1[F("type")] = (int)MODE_RAIL;
    br1[F("id")] = "";
    br1[F("name")] = "";

    // 2. TfL Tube
    if (MAX_BOARDS > 1) {
        JsonObject br2 = boards.add<JsonObject>();
        br2[F("type")] = (int)MODE_TUBE;
        br2[F("id")] = "";
        br2[F("name")] = "";
    }

    // 3. Bus
    if (MAX_BOARDS > 2) {
        JsonObject br3 = boards.add<JsonObject>();
        br3[F("type")] = (int)MODE_BUS;
        br3[F("id")] = "";
        br3[F("name")] = "";
    }

    // Additional slots as Rail boards if MAX_BOARDS > 3
    for (int i = 3; i < MAX_BOARDS; i++) {
        JsonObject br = boards.add<JsonObject>();
        br[F("type")] = (int)MODE_RAIL;
        br[F("id")] = "";
        br[F("name")] = "";
    }

    String output;
    serializeJson(doc, output);
    saveFile(F("/config.json"), output);
    
    // Clear in-memory stubs
    config.crsCode[0] = '\0';
    config.tubeId[0] = '\0';
    config.busId[0] = '\0';
    config.boardCount = (MAX_BOARDS < 6) ? MAX_BOARDS : 6;
}

/**
 * @brief Writes the current in-memory configuration to `/config.json` in the modern multi-board format.
 * @return True if save succeeded.
 */
bool ConfigManager::save() {
    LOG_INFO("CONFIG", "Saving configuration to /config.json...");
    JsonDocument doc;
    
    doc[F("version")] = 2.1;
    doc[F("hostname")] = config.hostname;
    doc[F("noScroll")] = config.noScrolling;
    doc[F("flip")] = config.flipScreen;
    doc[F("brightness")] = config.brightness;
    doc[F("sleep")] = config.sleepEnabled;
    doc[F("sleepStarts")] = config.sleepStarts;
    doc[F("sleepEnds")] = config.sleepEnds;
    doc[F("TZ")] = config.timezone;
    doc[F("showDate")] = config.dateEnabled;
    doc[F("clock")] = config.showClockInSleep;
    doc[F("fastRefresh")] = (config.apiRefreshRate == FASTDATAUPDATEINTERVAL);
    doc[F("update")] = config.firmwareUpdatesEnabled;
    doc[F("updateDaily")] = config.dailyUpdateCheckEnabled;
    doc[F("rssUrl")] = config.rssUrl;
    doc[F("rssName")] = config.rssName;
    doc[F("mode")] = config.defaultBoardIndex;

    JsonArray boards = doc[F("boards")].to<JsonArray>();
    for (int i = 0; i < config.boardCount; i++) {
        const BoardConfig& bc = config.boards[i];
        JsonObject b = boards.add<JsonObject>();
        b[F("type")] = (int)bc.type;
        b[F("id")] = bc.id;
        b[F("name")] = bc.name;
        b[F("lat")] = bc.lat;
        b[F("lon")] = bc.lon;
        b[F("filter")] = bc.filter;
        b[F("secId")] = bc.secondaryId;
        b[F("secName")] = bc.secondaryName;
        b[F("offset")] = bc.timeOffset;
        b[F("weather")] = bc.showWeather;
    }

    String output;
    serializeJson(doc, output);
    return saveFile(F("/config.json"), output);
}

/**
 * @brief Load all user preferences and module settings from `/config.json`.
 *        Supports migration from legacy flat format and provides stubs for legacy UI.
 */
void ConfigManager::loadConfig() {
  LOG_INFO("CONFIG", "Loading configuration from /config.json...");
  JsonDocument doc;

  // --- Step 1: Set Core Defaults ---
  strlcpy(config.hostname, "DeparturesBoard", sizeof(config.hostname));
  strlcpy(config.timezone, TimeManager::ukTimezone, sizeof(config.timezone));
  float loadedVersion = 1.0f; // Default for legacy detection if no version field
  config.boardCount = 0;

  if (LittleFS.exists(F("/config.json"))) {
    String contents = loadFile(F("/config.json"));
    LOG_DEBUG("CONFIG", String("Dumping /config.json:\n") + contents);

    DeserializationError error = deserializeJson(doc, contents);
    if (!error) {
        JsonObject settings = doc.as<JsonObject>();
        
        // Mode/Default Display selection
        if (settings[F("mode")].is<int>()) {
            config.defaultBoardIndex = settings[F("mode")];
        }

        // System settings
        if (settings[F("version")].is<float>())          loadedVersion = settings[F("version")];
        config.configVersion = loadedVersion; // Store what we found (or the default)
        if (settings[F("hostname")].is<const char*>()) strlcpy(config.hostname, settings[F("hostname")], sizeof(config.hostname));
        if (settings[F("noScroll")].is<bool>())          config.noScrolling = settings[F("noScroll")];
        if (settings[F("flip")].is<bool>())              config.flipScreen = settings[F("flip")];
        if (settings[F("brightness")].is<int>())         config.brightness = settings[F("brightness")];
        if (settings[F("sleep")].is<bool>())             config.sleepEnabled = settings[F("sleep")];
        if (settings[F("sleepStarts")].is<int>())        config.sleepStarts = settings[F("sleepStarts")];
        if (settings[F("sleepEnds")].is<int>())          config.sleepEnds = settings[F("sleepEnds")];
        if (settings[F("TZ")].is<const char*>())         strlcpy(config.timezone, settings[F("TZ")], sizeof(config.timezone));
        
        if (settings[F("showDate")].is<bool>())          config.dateEnabled = settings[F("showDate")];
        if (settings[F("clock")].is<bool>())             config.showClockInSleep = settings[F("clock")];
        if (settings[F("fastRefresh")].is<bool>())       config.apiRefreshRate = settings[F("fastRefresh")] ? FASTDATAUPDATEINTERVAL : DATAUPDATEINTERVAL;
        if (settings[F("update")].is<bool>())            config.firmwareUpdatesEnabled = settings[F("update")];
        if (settings[F("updateDaily")].is<bool>())       config.dailyUpdateCheckEnabled = settings[F("updateDaily")];

        // RSS
        if (settings[F("rssUrl")].is<const char*>())     strlcpy(config.rssUrl, settings[F("rssUrl")], sizeof(config.rssUrl));
        if (settings[F("rssName")].is<const char*>())    strlcpy(config.rssName, settings[F("rssName")], sizeof(config.rssName));
        config.rssEnabled = (config.rssUrl[0] != '\0');

        // --- Step 2: Provision Boards ---
        if (settings[F("boards")].is<JsonArray>()) {
            // --- Modern Nested Format ---
            JsonArray boards = settings[F("boards")].as<JsonArray>();
            config.boardCount = 0;
            for (JsonObject b : boards) {
                if (config.boardCount >= MAX_BOARDS) break;
                BoardConfig& bc = config.boards[config.boardCount++];
                
                // v2.1 Migration: "mode" renamed to "type"
                if (loadedVersion < 2.1f && b[F("mode")].is<int>()) {
                    bc.type = (BoardTypes)b[F("mode")].as<int>();
                } else {
                    bc.type = (BoardTypes)(b[F("type")] | 0);
                }

                strlcpy(bc.id, b[F("id")] | "", sizeof(bc.id));
                strlcpy(bc.name, b[F("name")] | "", sizeof(bc.name));
                bc.lat = b[F("lat")] | 0.0f;
                bc.lon = b[F("lon")] | 0.0f;
                strlcpy(bc.filter, b[F("filter")] | "", sizeof(bc.filter));
                strlcpy(bc.secondaryId, b[F("secId")] | "", sizeof(bc.secondaryId));
                strlcpy(bc.secondaryName, b[F("secName")] | "", sizeof(bc.secondaryName));
                bc.timeOffset = b[F("offset")] | 0;
                
                // Weather Toggle: Default to true unless it's a Tube board (legacy behavior)
                if (b[F("weather")].is<bool>()) {
                    bc.showWeather = b[F("weather")];
                } else {
                    bc.showWeather = (bc.type != MODE_TUBE);
                }
                
                LOG_INFO("SYSTEM", "Loaded Config: Board " + String(config.boardCount-1) + " Type=" + String((int)bc.type) + " ID=" + String(bc.id) + " Weather=" + String(bc.showWeather ? "ON" : "OFF"));
            }
            LOG_INFO("CONFIG", "Loaded " + String(config.boardCount) + " boards from modern config format.");
        } else {
            // --- Legacy Migration ---
            LOG_INFO("CONFIG", "Migrating legacy config format...");
            // Slot 0: Main Rail
            if (config.boardCount < MAX_BOARDS) {
                BoardConfig& br = config.boards[config.boardCount++];
                br.type = MODE_RAIL;
                strlcpy(br.id, settings[F("crs")] | "", sizeof(br.id));
                // Try multiple legacy keys for coordinates
                br.lat = settings[F("lat")] | settings[F("stationLat")] | 0.0f;
                br.lon = settings[F("lon")] | settings[F("stationLon")] | 0.0f;
                strlcpy(br.secondaryId, settings[F("callingCrs")] | "", sizeof(br.secondaryId));
                strlcpy(br.secondaryName, settings[F("callingStation")] | "", sizeof(br.secondaryName));
                strlcpy(br.filter, settings[F("platformFilter")] | "", sizeof(br.filter));
                br.timeOffset = settings[F("nrTimeOffset")] | 0;
            }

            // Slot 1: Tube
            if (config.boardCount < MAX_BOARDS) {
                BoardConfig& bt = config.boards[config.boardCount++];
                bt.type = MODE_TUBE;
                strlcpy(bt.id, settings[F("tubeId")] | "", sizeof(bt.id));
                strlcpy(bt.name, settings[F("tubeName")] | "", sizeof(bt.name));
            }

            // Slot 2: Bus
            if (config.boardCount < MAX_BOARDS) {
                BoardConfig& bb = config.boards[config.boardCount++];
                bb.type = MODE_BUS;
                strlcpy(bb.id, settings[F("busId")] | "", sizeof(bb.id));
                strlcpy(bb.name, settings[F("busName")] | "", sizeof(bb.name));
                bb.lat = settings[F("busLat")] | 0.0f;
                bb.lon = settings[F("busLon")] | 0.0f;
                strlcpy(bb.filter, settings[F("busFilter")] | "", sizeof(bb.filter));
            }

            // Slot 3: Alt Rail
            if (settings[F("altCrs")].is<const char*>() && config.boardCount < MAX_BOARDS) {
                BoardConfig& ba = config.boards[config.boardCount++];
                ba.type = MODE_RAIL;
                strlcpy(ba.id, settings[F("altCrs")], sizeof(ba.id));
                ba.lat = settings[F("altLat")] | 0.0f;
                ba.lon = settings[F("altLon")] | 0.0f;
                strlcpy(ba.secondaryId, settings[F("altCallingCrs")] | "", sizeof(ba.secondaryId));
                strlcpy(ba.secondaryName, settings[F("altCallingStation")] | "", sizeof(ba.secondaryName));
                strlcpy(ba.filter, settings[F("altPlatformFilter")] | "", sizeof(ba.filter));
                
                // Also sync to legacy stubs
                config.altStationEnabled = true;
                strlcpy(config.altCrsCode, settings[F("altCrs")], sizeof(config.altCrsCode));
                config.altLat = ba.lat;
                config.altLon = ba.lon;
                strlcpy(config.altCallingCrsCode, ba.secondaryId, sizeof(config.altCallingCrsCode));
                strlcpy(config.altCallingStation, ba.secondaryName, sizeof(config.altCallingStation));
                strlcpy(config.altPlatformFilter, ba.filter, sizeof(config.altPlatformFilter));
            }
        }

        // --- Step 3: Sync Legacy Stubs (Compatibility) ---
        // Post-load, we populate the legacy flat fields from Slot 0-2 to ensure
        // existing code (e.g. status pages, web UI) still sees data.
        if (config.boardCount > 0) {
            const BoardConfig& b0 = config.boards[0];
            config.boardType = b0.type;
            strlcpy(config.crsCode, b0.id, sizeof(config.crsCode));
            config.stationLat = b0.lat;
            config.stationLon = b0.lon;
            strlcpy(config.callingCrsCode, b0.secondaryId, sizeof(config.callingCrsCode));
            strlcpy(config.callingStation, b0.secondaryName, sizeof(config.callingStation));
            strlcpy(config.platformFilter, b0.filter, sizeof(config.platformFilter));
            config.nrTimeOffset = b0.timeOffset;
        }

        // Logic for identifying if one of the boards is the legacy "Alt Station"
        config.altStationEnabled = false;
        for (int i = 1; i < config.boardCount; i++) {
            if (config.boards[i].type == MODE_RAIL) {
                config.altStationEnabled = true;
                strlcpy(config.altCrsCode, config.boards[i].id, sizeof(config.altCrsCode));
                config.altLat = config.boards[i].lat;
                config.altLon = config.boards[i].lon;
                strlcpy(config.altCallingCrsCode, config.boards[i].secondaryId, sizeof(config.altCallingCrsCode));
                strlcpy(config.altCallingStation, config.boards[i].secondaryName, sizeof(config.altCallingStation));
                strlcpy(config.altPlatformFilter, config.boards[i].filter, sizeof(config.altPlatformFilter));
                break; 
            }
        }

        // TfL Stub (Slot 1 by convention in migration/default)
        if (config.boardCount > 1) {
            strlcpy(config.tubeId, config.boards[1].id, sizeof(config.tubeId));
            strlcpy(config.tubeName, config.boards[1].name, sizeof(config.tubeName));
        }

        // Bus Stub (Slot 2 by convention)
        if (config.boardCount > 2) {
            strlcpy(config.busId, config.boards[2].id, sizeof(config.busId));
            strlcpy(config.busName, config.boards[2].name, sizeof(config.busName));
            config.busLat = config.boards[2].lat;
            config.busLon = config.boards[2].lon;
            strlcpy(config.busFilter, config.boards[2].filter, sizeof(config.busFilter));
        }

        timeManager.setTimezone(String(config.timezone));
        LOG_INFO("CONFIG", "Configuration loaded. Board count: " + String(config.boardCount) + " Default board: " + String(config.defaultBoardIndex));
        
        // v2.1 Migration: Auto-save if version was upgraded
        if (loadedVersion < 2.1f) {
            LOG_INFO("CONFIG", "Auto-saving updated configuration (v2.1)...");
            save();
        }

        validate(); // Recalculate board readiness
      } else {
        LOG_ERROR("CONFIG", String("Failed to parse /config.json: ") + error.c_str());
      }
  } else {
    LOG_WARN("CONFIG", "/config.json not found. Creating default config.");
    writeDefaultConfig();
  }
}

/**
 * @brief Validates all configured boards and updates their 'complete' and 'errorType' properties.
 */
void ConfigManager::validate() {
    LOG_INFO("CONFIG", "Validating Board Configurations...");
    for (int i = 0; i < config.boardCount; i++) {
        BoardConfig& bc = config.boards[i];
        bc.complete = false;
        bc.errorType = 0;

        switch (bc.type) {
            case MODE_RAIL:
                if (strlen(config.nrToken) == 0) {
                    bc.errorType = 1; // Missing Key
                } else if (strlen(bc.id) == 0) {
                    bc.errorType = 2; // Missing ID
                } else {
                    bc.complete = true;
                }
                break;
            case MODE_TUBE:
                if (strlen(config.tflAppkey) == 0) {
                    bc.errorType = 1; // Missing Key
                } else if (strlen(bc.id) == 0) {
                    bc.errorType = 2; // Missing ID
                } else {
                    bc.complete = true;
                }
                break;
            case MODE_BUS:
                if (strlen(bc.id) == 0) {
                    bc.errorType = 2; // Missing ID
                } else {
                    bc.complete = true;
                }
                break;
        }
        if (bc.complete) {
            LOG_INFO("SYSTEM", "Board " + String(i) + " Validation: READY (Err=" + String(bc.errorType) + ")");
        } else {
            LOG_WARN("SYSTEM", "Board " + String(i) + " Validation: INCOMPLETE (Err=" + String(bc.errorType) + ")");
        }
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
