/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons
 * Attribution-NonCommercial-ShareAlike 4.0 International. To view a copy of
 * this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/configManager/configManager.cpp
 * Description: Implementation of configuration lifecycle methods including
 * LittleFS persistence and JSON serialization/deserialization.
 *
 * Exported Functions/Classes:
 * - ConfigManager::loadConfig: Load settings from /config.json.
 * - ConfigManager::saveConfig: Persist active settings to /config.json.
 * - ConfigManager::loadApiKeys: Load secrets from /apikeys.json.
 * - ConfigManager::saveApiKeys: Persist secrets to /apikeys.json.
 * - ConfigManager::writeDefaultConfig: Generate skeleton configuration.
 * - ConfigManager::saveFile: LittleFS write wrapper.
 * - ConfigManager::loadFile: LittleFS read wrapper.
 */

#include "configManager.hpp"
#include "gadecMigration.hpp"
#include <LittleFS.h>
#include <logger.hpp>
#include <timeManager.hpp>
#include <deviceCrypto.hpp>

// Forward declaration of the configuration subscriber interface
#include "iConfigurable.hpp"

/**
 * @brief Utility to save string data to a file in LittleFS.
 * @param fName Absolute path to the destination file.
 * @param fData Content string to write.
 * @return True if write was successful.
 */
bool ConfigManager::saveFile(String fName, String fData) {
  LOG_INFO("CONFIG", String("Attempting to save ") + fData.length() +
                         " bytes to file: " + fName);

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
  File f = LittleFS.open(fName, "r");
  if (f) {
    String result = f.readString();
    f.close();
    return result;
  } else
    return "";
}

/**
 * @brief Load sensitive tokens (NR token, OWM token, TfL App Key) from
 * `/apikeys.json`.
 */
void ConfigManager::loadApiKeys() {
  LOG_INFO("CONFIG", "Querying local API registry storage...");
  JsonDocument doc;

  if (LittleFS.exists("/apikeys.json")) {
    LOG_INFO("CONFIG", "Legacy plaintext `/apikeys.json` detected. Executing secure token migration...");
    String contents = loadFile("/apikeys.json");

    DeserializationError error = deserializeJson(doc, contents);
    if (!error) {
      // --- Step 1: Migration Check ---
      // Validate the JSON version and handle legacy field mapping if needed.
      JsonObject root = doc.as<JsonObject>();
      float version = root[F("version")] | 1.0f;

      if (version < 2.0f) {
        // --- Migration v1.0 -> v2.0 ---
        LOG_INFO("CONFIG",
                 "Migrating /apikeys.json to v2.0 registry format...");
        config.keyCount = 0;

        if (root[F("nrToken")].is<const char *>()) {
          ApiKey &k = config.keys[config.keyCount++];
          strlcpy(k.id, "k-nr-legacy", sizeof(k.id));
          strlcpy(k.label, "Default Rail Key", sizeof(k.label));
          strlcpy(k.type, "rail", sizeof(k.type));
          strlcpy(k.token, root[F("nrToken")], sizeof(k.token));
        }

        if (root[F("appKey")].is<const char *>()) {
          ApiKey &k = config.keys[config.keyCount++];
          strlcpy(k.id, "k-tfl-legacy", sizeof(k.id));
          strlcpy(k.label, "Default TfL Key", sizeof(k.label));
          strlcpy(k.type, "tfl", sizeof(k.type));
          strlcpy(k.token, root[F("appKey")], sizeof(k.token));
        }

        if (root[F("owmToken")].is<const char *>()) {
          ApiKey &k = config.keys[config.keyCount++];
          strlcpy(k.id, "k-owm-legacy", sizeof(k.id));
          strlcpy(k.label, "Default Weather Key", sizeof(k.label));
          strlcpy(k.type, "owm", sizeof(k.type));
          strlcpy(k.token, root[F("owmToken")], sizeof(k.token));
        }

        config.apiKeysLoaded = true;
      } else {
        // --- Load v2.0 Format ---
        JsonArray keys = root[F("keys")].as<JsonArray>();
        config.keyCount = 0;
        for (JsonObject kObj : keys) {
          if (config.keyCount >= MAX_KEYS)
            break;
          ApiKey &k = config.keys[config.keyCount++];
          strlcpy(k.id, kObj[F("id")] | "", sizeof(k.id));
          strlcpy(k.label, kObj[F("label")] | "", sizeof(k.label));
          strlcpy(k.type, kObj[F("type")] | "", sizeof(k.type));
          strlcpy(k.token, kObj[F("token")] | "", sizeof(k.token));
        }
        config.apiKeysLoaded = true;
      }

      // Explicitly persist back to LittleFS as encrypted blob and drop old variant
      saveApiKeys();
      LittleFS.remove("/apikeys.json");
      LOG_INFO("CONFIG", "Legacy API credentials safely migrated and eradicated for security.");

      // Register secrets for redaction
      for (int i = 0; i < config.keyCount; i++) {
        if (config.keys[i].token[0]) {
          LOG_REGISTER_SECRET(config.keys[i].token);
        }
      }

    } else {
      LOG_ERROR("CONFIG",
                String("Failed to parse /apikeys.json: ") + error.c_str());
    }
  } else if (LittleFS.exists("/apikeys.bin")) {
     if (cryptoEngine) {
       File f = LittleFS.open("/apikeys.bin", "r");
       if (f) {
           size_t fSize = f.size();
           std::unique_ptr<char[]> fileBuf(new (std::nothrow) char[fSize + 1]);
           if (fileBuf) {
               f.readBytes(fileBuf.get(), fSize);
               fileBuf[fSize] = '\0';
               
               size_t plainLen = 0;
               std::unique_ptr<char[]> decryptedRaw = cryptoEngine->decrypt(fileBuf.get(), fSize, &plainLen);
               if (decryptedRaw && plainLen > 0) {
                    DeserializationError error = deserializeJson(doc, decryptedRaw.get());
                    if (!error) {
                         JsonObject root = doc.as<JsonObject>();
                         JsonArray keys = root[F("keys")].as<JsonArray>();
                         config.keyCount = 0;
                         for (JsonObject kObj : keys) {
                           if (config.keyCount >= MAX_KEYS) break;
                           ApiKey &k = config.keys[config.keyCount++];
                           strlcpy(k.id, kObj[F("id")] | "", sizeof(k.id));
                           strlcpy(k.label, kObj[F("label")] | "", sizeof(k.label));
                           strlcpy(k.type, kObj[F("type")] | "", sizeof(k.type));
                           strlcpy(k.token, kObj[F("token")] | "", sizeof(k.token));
                         }
                         config.apiKeysLoaded = true;
                         
                         // Register secrets for redaction
                         for (int i = 0; i < config.keyCount; i++) {
                           if (config.keys[i].token[0]) {
                             LOG_REGISTER_SECRET(config.keys[i].token);
                           }
                         }
                         LOG_INFOf("SYSTEM", "API Key Registry securely loaded with %d keys.", config.keyCount);
                    } else {
                       LOG_ERROR("CONFIG", "Decrypted API token failure. Keys misaligned?");
                    }
               }
           }
           f.close();
       }
     } else {
        LOG_ERROR("CONFIG", "Security Engine unbound. Cannot decrypt API tokens.");
     }
  } else {
    // Neither exist
    LOG_WARN("CONFIG", "No API tokens found on storage.");
  }
}

/**
 * @brief Writes the current Key Registry to secured binary blob.
 */
bool ConfigManager::saveApiKeys() {
  LOG_INFO("CONFIG", "Securing API Key Registry...");
  if (!cryptoEngine) {
     LOG_ERROR("CONFIG", "Security Engine unbound. Aborting API key save.");
     return false;
  }

  // --- Step 1: Package Registry into JSON ---
  JsonDocument doc;
  doc[F("version")] = 2.0;
  JsonArray keys = doc[F("keys")].to<JsonArray>();

  for (int i = 0; i < config.keyCount; i++) {
    JsonObject kObj = keys.add<JsonObject>();
    kObj[F("id")] = config.keys[i].id;
    kObj[F("label")] = config.keys[i].label;
    kObj[F("type")] = config.keys[i].type;
    kObj[F("token")] = config.keys[i].token;
  }

  size_t jsonLen = measureJson(doc);
  std::unique_ptr<char[]> plainBuf(new (std::nothrow) char[jsonLen + 1]);
  if (!plainBuf) {
      LOG_ERROR("CONFIG", "Serialization memory allocation failed.");
      return false;
  }
  serializeJson(doc, plainBuf.get(), jsonLen + 1);
  
  // --- Step 2: Encrypt Payload ---
  size_t cipherLen = 0;
  std::unique_ptr<char[]> securedBlob = cryptoEngine->encrypt(plainBuf.get(), jsonLen, &cipherLen);
  if (!securedBlob || cipherLen == 0) {
      LOG_ERROR("CONFIG", "Cryptographic API token serialization failed.");
      return false;
  }
  
  // --- Step 3: Write to Secure Binary File ---
  File f = LittleFS.open(F("/apikeys.bin"), "w");
  if (!f) {
      LOG_ERROR("CONFIG", "Failed to construct secure binary file.");
      return false;
  }
  f.write((uint8_t*)securedBlob.get(), cipherLen);
  f.close();
  return true;
}

/**
 * @brief Adds or updates a key in the registry.
 */
void ConfigManager::updateKey(const ApiKey &key) {
  ApiKey *existing = getKeyById(key.id);
  if (existing) {
    memcpy(existing, &key, sizeof(ApiKey));
  } else if (config.keyCount < MAX_KEYS) {
    memcpy(&config.keys[config.keyCount++], &key, sizeof(ApiKey));
  }
  saveApiKeys();
}

/**
 * @brief Removes a key from the registry by ID.
 */
void ConfigManager::deleteKey(const char *id) {
  for (int i = 0; i < config.keyCount; i++) {
    if (strcmp(config.keys[i].id, id) == 0) {
      // Shift remaining keys
      for (int j = i; j < config.keyCount - 1; j++) {
        memcpy(&config.keys[j], &config.keys[j + 1], sizeof(ApiKey));
      }
      config.keyCount--;
      saveApiKeys();
      return;
    }
  }
}

/**
 * @brief Retrieves a key from the registry by ID.
 */
ApiKey *ConfigManager::getKeyById(const char *id) {
  if (!id || id[0] == '\0')
    return nullptr;
  for (int i = 0; i < config.keyCount; i++) {
    if (strcmp(config.keys[i].id, id) == 0)
      return &config.keys[i];
  }
  return nullptr;
}

/**
 * @brief Write a default configuration file with 4 standard boards.
 */
void ConfigManager::writeDefaultConfig() {
  LOG_INFO("CONFIG", "Generating default configuration...");
  JsonDocument doc;

  doc[F("hostname")] = "DeparturesBoard";
  doc[F("version")] = (float)CONFIG_VERSION_MAJOR + ((float)CONFIG_VERSION_MINOR / 10.0f); 
  doc[F("brightness")] = 20;
  doc[F("showDate")] = false;
  doc[F("flip")] = false;
  doc[F("TZ")] = TimeManager::ukTimezone;
  doc[F("overrideTimeout")] = 60;
  doc[F("overrideTimeout")] = 60;
  doc[F("carouselInterval")] = 120;

  JsonArray schedules = doc[F("schedules")].to<JsonArray>();
  for (int i = 0; i < MAX_SCHEDULE_RULES; i++) {
    JsonObject s = schedules.add<JsonObject>();
    s[F("startH")] = 0;
    s[F("startM")] = 0;
    s[F("endH")] = 23;
    s[F("endM")] = 59;
    s[F("board")] = -1;
  }

  JsonArray boards = doc[F("boards")].to<JsonArray>();

  // No active display boards are provisioned automatically.
  // The system will natively pause in BOARD_SETUP and display the Help Overlay screen 
  // until the user adds their first board via the Web Portal.

  String output;
  serializeJson(doc, output);
  saveFile(getActiveConfigFilename(), output);

  config.boardCount = 0;
}

/**
 * @brief Writes the current in-memory configuration to `/config.json` in the
 * modern multi-board format.
 * @return True if save succeeded.
 */
bool ConfigManager::save() {
  String fName = getActiveConfigFilename();
  LOG_INFO("CONFIG", "Saving configuration to " + fName + "...");
  JsonDocument doc;

  doc[F("version")] = (float)CONFIG_VERSION_MAJOR + ((float)CONFIG_VERSION_MINOR / 10.0f);
#ifdef BUILD_TIME
  doc[F("build")] = BUILD_TIME;
#endif
  doc[F("hostname")] = config.hostname;
  doc[F("noScroll")] = config.noScrolling;
  doc[F("flip")] = config.flipScreen;
  doc[F("brightness")] = config.brightness;
  doc[F("TZ")] = config.timezone;
  doc[F("showDate")] = config.dateEnabled;
  doc[F("fastRefresh")] = (config.apiRefreshRate == FASTDATAUPDATEINTERVAL);
  doc[F("waitForScroll")] = config.waitForScrollComplete;
  doc[F("rssFirst")] = config.prioritiseRss;
  doc[F("update")] = config.firmwareUpdatesEnabled;
  doc[F("updateDaily")] = config.dailyUpdateCheckEnabled;
  doc[F("otaQuietHour")] = config.otaQuietHour;

  JsonObject feeds = doc[F("feeds")].to<JsonObject>();
  feeds[F("rss")] = config.rssUrl;
  feeds[F("rssName")] = config.rssName;
  feeds[F("weatherKeyId")] = config.weatherKeyId;

  doc[F("overrideTimeout")] = config.manualOverrideTimeoutSecs;
  doc[F("carouselInterval")] = config.carouselIntervalSecs;

  JsonArray schedules = doc[F("schedules")].to<JsonArray>();
  for (int i = 0; i < MAX_SCHEDULE_RULES; i++) {
    const ScheduleRule &r = config.schedules[i];
    if (r.boardIndex != -1) {
      JsonObject s = schedules.add<JsonObject>();
      s[F("startH")] = r.startHour;
      s[F("startM")] = r.startMinute;
      s[F("endH")] = r.endHour;
      s[F("endM")] = r.endMinute;
      s[F("board")] = r.boardIndex;
    }
  }

  JsonArray boards = doc[F("boards")].to<JsonArray>();
  for (int i = 0; i < config.boardCount; i++) {
    const BoardConfig &bc = config.boards[i];
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
    b[F("brightness")] = bc.brightness;
    b[F("apiKeyId")] = bc.apiKeyId;
    b[F("tflLine")] = bc.tflLineFilter;
    b[F("tflDir")] = bc.tflDirectionFilter;
    b[F("ordinals")] = bc.showServiceOrdinals;
    b[F("lastSeen")] = bc.showLastSeenLocation;
    b[F("oledOff")] = bc.oledOff;
    b[F("layout")] = bc.layout;
  }

  String output;
  serializeJson(doc, output);
  bool success = saveFile(fName, output);

  if (success) {
    // --- Step 4: Pruning Strategy (Keep 3 + config.json) ---
    // Enumerate existing config_X_Y.json files and keep only the 3 most recent
    struct ConfigFile {
        String name;
        int major;
        int minor;
    };
    std::vector<ConfigFile> files;

    File root = LittleFS.open("/", "r");
    if (root && root.isDirectory()) {
        File f = root.openNextFile();
        while (f) {
            String name = f.name();
            if (name.startsWith("/config_") && name.endsWith(".json")) {
                // Remove leading slash if present
                String baseName = name.startsWith("/") ? name.substring(1) : name;
                int firstUnderscore = baseName.indexOf('_');
                int lastUnderscore = baseName.lastIndexOf('_');
                int dot = baseName.indexOf('.');
                if (firstUnderscore != -1 && lastUnderscore != -1 && dot != -1) {
                    int maj = baseName.substring(firstUnderscore + 1, lastUnderscore).toInt();
                    int min = baseName.substring(lastUnderscore + 1, dot).toInt();
                    files.push_back({baseName, maj, min});
                }
            }
            f = root.openNextFile();
        }
    }

    // Sort by version descending
    std::sort(files.begin(), files.end(), [](const ConfigFile& a, const ConfigFile& b) {
        if (a.major != b.major) return a.major > b.major;
        return a.minor > b.minor;
    });

    // Delete beyond the top 3
    if (files.size() > 3) {
        for (size_t i = 3; i < files.size(); i++) {
            LOG_INFO("CONFIG", "Pruning old configuration: " + files[i].name);
            LittleFS.remove("/" + files[i].name);
        }
    }
  }

  return success;
}

/**
 * @brief Load all user preferences and module settings from `/config.json`.
 *        Supports migration from legacy flat format.
 */
void ConfigManager::loadConfig() {
  LOG_INFO("CONFIG", "Initiating multi-tier configuration hunt...");
  JsonDocument doc;

  // --- Step 1: Set Core Defaults ---
  strlcpy(config.hostname, "DeparturesBoard", sizeof(config.hostname));
  strlcpy(config.timezone, TimeManager::ukTimezone, sizeof(config.timezone));
  config.boardCount = 0;
  config.configVersion = (float)CONFIG_VERSION_MAJOR + ((float)CONFIG_VERSION_MINOR / 10.0f);

  // --- Step 2: Version Hunting ---
  String targetFile = "";
  String activeFile = getActiveConfigFilename();
  LOG_INFO("CONFIG", "Primary target: " + activeFile);

  if (LittleFS.exists(activeFile)) {
      targetFile = activeFile;
      LOG_INFO("CONFIG", "Native versioned config found. Skipping migration hunt.");
  } else {
      LOG_INFO("CONFIG", "Native config missing. Scanning filesystem for legacy fallbacks...");
      // Find most recent fallback
      File root = LittleFS.open("/", "r");
      float maxVer = 0.0f;
      if (root && root.isDirectory()) {
          File f = root.openNextFile();
          while (f) {
              String name = f.name();
              // Remove leading slash if present for parsing
              String baseName = name.startsWith("/") ? name.substring(1) : name;
              if (baseName.startsWith("config_") && baseName.endsWith(".json")) {
                  int firstUnderscore = baseName.indexOf('_');
                  int lastUnderscore = baseName.lastIndexOf('_');
                  int dot = baseName.indexOf('.');
                  if (firstUnderscore != -1 && lastUnderscore != -1 && dot != -1) {
                      float v = baseName.substring(firstUnderscore + 1, lastUnderscore).toFloat() + 
                                (baseName.substring(lastUnderscore + 1, dot).toFloat() / 10.0f);
                      if (v > maxVer) {
                          maxVer = v;
                          targetFile = (name.startsWith("/") ? name : "/" + name);
                      }
                  }
              }
              f = root.openNextFile();
          }
      }
      
      if (targetFile == "" && LittleFS.exists("/config.json")) {
          targetFile = "/config.json";
      }
  }

  if (targetFile != "") {
    LOG_INFO("CONFIG", "Acquiring configuration from: " + targetFile);
    String contents = loadFile(targetFile);

    DeserializationError error = deserializeJson(doc, contents);
    if (!error) {
      JsonObject settings = doc.as<JsonObject>();
      float loadedVersion = settings[F("version")] | 1.0f;

      GadecMigration::UpstreamEpoch epoch = GadecMigration::detectConfigEpoch(settings);

      // --- Suffix-Renaming Policy for Unversioned Fork Configs ---
      // If we loaded from the base '/config.json' but the file contains a version field,
      // it means this is a project-native file that hasn't been renamed yet.
      if (targetFile == "/config.json" && settings.containsKey("version")) {
          int major = (int)loadedVersion;
          int minor = (int)((loadedVersion - major) * 10.0f + 0.5f);
          String newName = "/config_" + String(major) + "_" + String(minor) + ".json";
          
          if (!LittleFS.exists(newName)) {
              LOG_INFO("CONFIG", "[MIGRATION] Normalizing unversioned fork config to: " + newName);
              LittleFS.rename("/config.json", newName);
          }
      }

      if (epoch != GadecMigration::EPOCH_LATEST_NATIVE) {
          LOG_WARN("CONFIG", "[MIGRATION] Legacy epoch detected. Shifting to Translation Matrix...");
          if (GadecMigration::translateToModern(doc, epoch)) {
              // Re-acquire settings object pointer after potential doc growth
              settings = doc.as<JsonObject>();
              loadedVersion = settings[F("version")] | 2.6f;
              
              // Force a save to the modern versioned file to solidify state
              LOG_INFO("CONFIG", "[MIGRATION] Migration successful. Persisting versioned state...");
              save(); 
          }
      }

      config.configVersion = loadedVersion;

      // Extract system settings
      if (settings[F("hostname")].is<const char *>())
        strlcpy(config.hostname, settings[F("hostname")],
                sizeof(config.hostname));
      if (settings[F("noScroll")].is<bool>())
        config.noScrolling = settings[F("noScroll")];
      if (settings[F("flip")].is<bool>())
        config.flipScreen = settings[F("flip")];
      if (settings[F("brightness")].is<int>())
        config.brightness = settings[F("brightness")];
      if (settings[F("TZ")].is<const char *>())
        strlcpy(config.timezone, settings[F("TZ")], sizeof(config.timezone));

      if (settings[F("showDate")].is<bool>())
        config.dateEnabled = settings[F("showDate")];
      if (settings[F("fastRefresh")].is<bool>())
        config.apiRefreshRate = settings[F("fastRefresh")]
                                    ? FASTDATAUPDATEINTERVAL
                                    : DATAUPDATEINTERVAL;
      if (settings[F("update")].is<bool>())
        config.firmwareUpdatesEnabled = settings[F("update")];
      if (settings[F("updateDaily")].is<bool>())
        config.dailyUpdateCheckEnabled = settings[F("updateDaily")];
      if (settings[F("otaQuietHour")].is<int>())
        config.otaQuietHour = settings[F("otaQuietHour")];
      if (settings[F("waitForScroll")].is<bool>())
        config.waitForScrollComplete = settings[F("waitForScroll")];
      if (settings[F("rssFirst")].is<bool>())
        config.prioritiseRss = settings[F("rssFirst")];

      // Scheduling and carousel defaults
      config.manualOverrideTimeoutSecs = settings[F("overrideTimeout")] | 60;
      config.carouselIntervalSecs = settings[F("carouselInterval")] | 120;

      if (settings[F("schedules")].is<JsonArray>()) {
        JsonArray schedules = settings[F("schedules")].as<JsonArray>();
        int count = 0;
        for (JsonObject s : schedules) {
          if (count >= MAX_SCHEDULE_RULES)
            break;
          ScheduleRule &r = config.schedules[count++];
          r.startHour = s[F("startH")] | 0;
          r.startMinute = s[F("startM")] | 0;
          r.endHour = s[F("endH")] | 23;
          r.endMinute = s[F("endM")] | 59;
          r.boardIndex = s[F("board")] | -1;
        }
        // Fill remaining if needed
        for (int i = count; i < MAX_SCHEDULE_RULES; i++) {
          config.schedules[i].boardIndex = -1;
        }
      }

      // v2.3 Migration: Global turnOffOledInSleep moved to per-board oledOff
      bool legacyOledOff = false;
      if (settings[F("turnOffOledInSleep")].is<bool>()) {
        legacyOledOff = settings[F("turnOffOledInSleep")];
      }

      // RSS (Legacy Fallback)
      if (settings[F("rssUrl")].is<const char *>())
        strlcpy(config.rssUrl, settings[F("rssUrl")], sizeof(config.rssUrl));
      if (settings[F("rssName")].is<const char *>())
        strlcpy(config.rssName, settings[F("rssName")], sizeof(config.rssName));
      if (settings[F("weatherKeyId")].is<const char *>())
        strlcpy(config.weatherKeyId, settings[F("weatherKeyId")],
                sizeof(config.weatherKeyId));

      // Feeds (Modern Nested Schema)
      if (settings[F("feeds")].is<JsonObject>()) {
        JsonObject f = settings[F("feeds")];
        if (f[F("rss")].is<const char *>())
          strlcpy(config.rssUrl, f[F("rss")], sizeof(config.rssUrl));
        if (f[F("rssName")].is<const char *>())
          strlcpy(config.rssName, f[F("rssName")], sizeof(config.rssName));
        if (f[F("weatherKeyId")].is<const char *>())
          strlcpy(config.weatherKeyId, f[F("weatherKeyId")], sizeof(config.weatherKeyId));
      }

      config.rssEnabled = (config.rssUrl[0] != '\0');

      // --- Step 2: Provision Boards ---
      if (settings[F("boards")].is<JsonArray>()) {
        // --- Modern Nested Format ---
        JsonArray boards = settings[F("boards")].as<JsonArray>();
        config.boardCount = 0;
        for (JsonObject b : boards) {
          if (config.boardCount >= MAX_BOARDS)
            break;
          BoardConfig &bc = config.boards[config.boardCount++];

          // v2.1 Migration: "mode" renamed to "type"
          if (loadedVersion < 2.1f && b[F("mode")].is<int>()) {
            bc.type = (BoardTypes)b[F("mode")].as<int>();
          } else {
            bc.type = (BoardTypes)(b[F("type")] | 0);
          }

          strlcpy(bc.id, b[F("id")].as<String>().c_str(), sizeof(bc.id));
          strlcpy(bc.name, b[F("name")].as<String>().c_str(), sizeof(bc.name));
          bc.lat = b[F("lat")] | 0.0f;
          bc.lon = b[F("lon")] | 0.0f;
          strlcpy(bc.filter, b[F("filter")].as<String>().c_str(),
                  sizeof(bc.filter));
          strlcpy(bc.secondaryId, b[F("secId")].as<String>().c_str(),
                  sizeof(bc.secondaryId));
          strlcpy(bc.secondaryName, b[F("secName")].as<String>().c_str(),
                  sizeof(bc.secondaryName));
          bc.timeOffset = b[F("offset")] | 0;
          bc.brightness = b[F("brightness")] | -1;
          strlcpy(bc.apiKeyId, b[F("apiKeyId")].as<String>().c_str(),
                  sizeof(bc.apiKeyId));
          strlcpy(bc.tflLineFilter, b[F("tflLine")].as<String>().c_str(),
                  sizeof(bc.tflLineFilter));
          bc.tflDirectionFilter = b[F("tflDir")] | 0;
          // Apply migration if board is a clock
          if (loadedVersion < 2.4f && legacyOledOff && bc.type == MODE_CLOCK) {
            bc.oledOff = true;
          } else {
            bc.oledOff = b[F("oledOff")] | false;
          }
          strlcpy(bc.layout, b[F("layout")].as<String>().c_str(),
                  sizeof(bc.layout));

          // Weather Toggle
          if (b[F("weather")].is<bool>()) {
            bc.showWeather = b[F("weather")];
          } else {
            bc.showWeather = (bc.type != MODE_TUBE);
          }

          LOG_INFO("SYSTEM", "Loaded Config: Board " +
                                 String(config.boardCount - 1) + " Type=" +
                                 String((int)bc.type) + " ID=" + String(bc.id));
        }
      }

      LOG_INFO("CONFIG", "Configuration loaded. Board count: " +
                             String(config.boardCount));
      validate(); // Recalculate board readiness
    } else {
      LOG_ERROR("CONFIG",
                String("CRITICAL CONFIGURATION FAULT: Failed to parse /config.json entirely. Error mode: [") + error.c_str() + "]. "
                "This typically indicates memory exhaustion (NoMemory), structurally flawed JSON syntax, or a file that could only be partially mapped/deserialized.");
      
      if (LittleFS.exists("/config.json.lastknowngood")) {
        LOG_WARN("CONFIG", "MITIGATION INITIATED: Configuration payload is corrupted or partially mapped. "
                           "Triggering transactional fallback. System will forcefully evacuate the tainted /config.json and restore from the trusted .lastknowngood baseline.");
        LittleFS.remove("/config.json");
        LittleFS.rename("/config.json.lastknowngood", "/config.json");
        LOG_WARN("CONFIG", "RESTORE IS SUCCESSFUL. Instigating recursive configuration boot sequence natively...");
        _rollbackFlag = true;
        loadConfig(); // Recursive reload to acquire safe dataset natively
        return;       // Terminate the tainted stack execution instantly
      }
    }

    // Always validate after loading to establish 'complete' flags
    validate();
  } else {
    LOG_WARN("CONFIG", "VITAL: Native /config.json not found on disk. Orchestrating a clean factory default installation...");
    writeDefaultConfig();
  }
}

/**
 * @brief Validates all configured boards and updates their 'complete' and
 * 'errorType' properties.
 */
void ConfigManager::validate() {
  LOG_INFO("CONFIG", "Validating Board Configurations...");
  for (int i = 0; i < config.boardCount; i++) {
    BoardConfig &bc = config.boards[i];
    bc.complete = false;
    bc.errorType = 0;

    switch (bc.type) {
    case MODE_RAIL:
      if (strlen(bc.id) == 0) {
        bc.errorType = 2; // Missing ID
      } else {
        bc.complete = true;
      }
      break;
    case MODE_TUBE:
      if (strlen(bc.id) == 0) {
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
    case MODE_CLOCK:
      bc.complete = true; // Clock is always valid
      break;
    }

    // --- Cross-Validation: API Key Registry Check ---
    if (bc.complete) {
      // Bus and Clock types do not require a network API key registry check.
      if (bc.type != MODE_BUS && bc.type != MODE_CLOCK) {
        ApiKey *key = getKeyById(bc.apiKeyId);
        if (!key) {
          bc.complete = false;
          bc.errorType = 1; // Missing/Invalid Key
        }
      }
    }

    if (bc.complete) {
      LOG_INFO("SYSTEM", "Board " + String(i) + " Validation: READY (Err=" +
                             String(bc.errorType) + ")");
    } else {
      LOG_WARN("SYSTEM", "Board " + String(i) +
                             " Validation: INCOMPLETE (Err=" +
                             String(bc.errorType) + ")");
    }
  }
}

/**
 * @brief Register a component to receive configuration updates.
 * @param consumer Pointer to the component to register.
 */
void ConfigManager::registerConsumer(iConfigurable *consumer) {
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
