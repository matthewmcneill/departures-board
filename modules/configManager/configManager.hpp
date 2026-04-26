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
 * Module: modules/configManager/configManager.hpp
 * Description: Manages the loading, saving, and parsing of application
 * configurations and API keys stored in JSON format on LittleFS.
 *
 * Exported Functions/Classes:
 * - BoardTypes: Enum defining supported operational modes (Rail, Tube, Bus,
 * Clock).
 * - Config: Struct containing all system and user application settings.
 * - ConfigManager: Class for handling configuration persistence and access.
 * - configManager: Global singleton for configuration management.
 */

#pragma once

#include <Arduino.h>

#define CONFIG_VERSION_MAJOR 2 // Primary structural epoch
#define CONFIG_VERSION_MINOR 6 // Nested feeds schema

#include "departuresBoard.hpp"

/**
 * @brief Supported operational types for the display boards.
 */
enum BoardTypes {
  MODE_RAIL = 0,
  MODE_TUBE = 1,
  MODE_BUS = 2,
  MODE_CLOCK = 3,
  MODE_DIAGNOSTIC = 4
};

/**
 * @brief Configuration for a single display board instance.
 */
struct BoardConfig {
  BoardTypes type = MODE_RAIL; // Type of board (Rail, Tube, Bus)
  char name[80] = "";          // Human-readable name for the station/stop
  char id[13] = "";       // ID (CRS for Rail, Naptan for Tube, Atco for Bus)
  char apiKeyId[13] = ""; // ID of the API key to use for this board

  // --- Location Settings ---
  float lat = 0.0f; // Latitude for weather/local info
  float lon = 0.0f; // Longitude for weather/local info

  // --- Filters & Filters ---
  char filter[54] = "";        // Platform filter or Bus route whitelist
  char secondaryId[13] = "";   // Calling station CRS or secondary filter
  char secondaryName[45] = ""; // Human name for calling filter

  // --- Timing ---
  int timeOffset = 0; // Local board time adjustment (NR specific)

  // --- Features ---
  bool showWeather = true; // Enable/Disable weather overlay for this board
  int brightness = -1;     // Per-board brightness override (-1 = use global)

  // --- Upstream B2.4-W3.1 Features ---
  char tflLineFilter[32] = "";       // Tube Line (e.g. "Central")
  int tflDirectionFilter = 0;        // 0: Any, 1: Inbound, 2: Outbound
  bool showServiceOrdinals = false;  // "2nd", "3rd" etc.
  bool showLastSeenLocation = false; // NR specific: "Last seen at..."
  bool oledOff = false; // Turn OLED off completely when this board is active
  char layout[32] = ""; // Target layout ID, defaults to empty (i.e., 'default')

  // --- Runtime Readiness (Computed by ConfigManager) ---
  bool complete = false; // True if all mandatory fields are present
  int errorType = 0;     // 0: OK, 1: Missing API Keys, 2: Missing Station ID
};

/**
 * @brief Represents a single API key in the registry.
 */
struct ApiKey {
  char id[13] = "";    // Unique ID for referencing (e.g. k-12345678)
  char label[32] = ""; // Human-readable label
  char type[12] = "";  // rail, tfl, owm
  char token[128] = ""; // The actual key/token
};

/**
 * @brief Represents a single time-based routing rule mapping a time block to a
 * display board.
 */
struct ScheduleRule {
  int startHour = 0;   // Start hour of the active period (0-23)
  int startMinute = 0; // Start minute of the active period (0-59)
  int endHour = 23;    // End hour of the active period (0-23)
  int endMinute = 59;  // End minute of the active period (0-59)
  int boardIndex = -1; // -1 indicates an empty/inactive rule slot
};

/**
 * @brief Container for all application configuration settings.
 */
struct Config {
  // --- API Key Registry ---
  ApiKey keys[MAX_KEYS]; // Registry of API keys
  int keyCount = 0;      // Number of keys in the registry
  bool apiKeysLoaded =
      false; // Flag indicating if secrets are successfully loaded

  // --- System Settings ---
  char hostname[33] = "DeparturesBoard";                    // Network hostname
  char wsdlHost[48] = "lite.realtime.nationalrail.co.uk";   // WSDL service host
  char wsdlAPI[48] = "/OpenLDBWS/wsdl.aspx?ver=2021-11-01"; // WSDL path
  char timezone[64] = "Europe/London"; // POSIX timezone string
  float configVersion =
      2.6f; // Configuration format version number (v2.6 nested feeds)

  // --- Display Preferences ---
  bool dateEnabled = false;     // Show date in header
  bool noScrolling = false;     // Disable panel animations
  bool hidePlatform = false;    // Suppress platform columns
  bool flipScreen = false;      // Rotate display 180 degrees
  int brightness = 20;          // Hardware brightness level (0-255)
  bool waitForScrollComplete =
      false;                  // Wait for message to finish before cycling board
  bool prioritiseRss = false; // Show RSS before service messages

  // --- Refresh & Update Settings ---
  long apiRefreshRate = DATAUPDATEINTERVAL; // Current polling interval
  bool firmwareUpdatesEnabled = false;      // Enable background OTA checks
  bool dailyUpdateCheckEnabled =
      false; // Check for firmware updates at midnight
  int otaQuietHour = 3; // Quiet hour for passive update checks (default 3 AM)

  // --- Dynamic Board List ---
  BoardConfig boards[MAX_BOARDS]; // Array of configured display boards
  int boardCount = 0;             // Number of active boards in the carousel

  // --- Schedule & Carousel Settings ---
  ScheduleRule schedules[MAX_SCHEDULE_RULES]; // Time-based routing arrays
  int manualOverrideTimeoutSecs =
      60; // How long to stay out of schedule when button is pressed
  int carouselIntervalSecs = 120; // Pace of rotation between active boards
  int defaultBoardIndex = 0;      // Index of the board to show at boot or when no schedule is active

  char rssUrl[128] = "";      // XML feed URL
  char rssName[48] = "";      // Label for news source
  bool rssEnabled = false;    // Enable headlines scroller
  char weatherKeyId[13] = ""; // Global weather API key ID
};

/**
 * @brief Class responsible for persistence and access of the Config object.
 */
class DeviceCrypto; // Forward declaration

class ConfigManager {
private:
  Config config; // Master configuration state container
  DeviceCrypto* cryptoEngine = nullptr; ///< Reference to hardware encryption provider
  bool _rollbackFlag = false; // Indicates if the configuration was forcibly rolled back during boot

public:
  ConfigManager() = default;

  /**
   * @brief Inject the crypto engine before initial loading
   */
  void bindCrypto(DeviceCrypto* crypto) { cryptoEngine = crypto; }

  bool hasRollback() const { return _rollbackFlag; }
  void clearRollback() { _rollbackFlag = false; }

  /**
   * @brief Construct the versioned configuration filename based on current macros.
   * @return String like "/config_2_6.json"
   */
  static String getActiveConfigFilename() {
    return String("/config_") + String(CONFIG_VERSION_MAJOR) + "_" + String(CONFIG_VERSION_MINOR) + ".json";
  }

  /**
   * @brief Load all user preferences and module settings from `/config.json`.
   */
  void loadConfig();

  /**
   * @brief Load sensitive tokens (NR token, OWM token, TfL App Key) from
   * `/apikeys.json`.
   */
  void loadApiKeys();

  /**
   * @brief Validates all configured boards and updates their 'complete' and
   * 'errorType' properties.
   */
  void validate();

  /**
   * @brief Writes a skeleton `/config.json` configuration onto LittleFS.
   */
  void writeDefaultConfig();

  /**
   * @brief Writes the current in-memory configuration to `/config.json` in the
   * modern multi-board format.
   * @return True if save succeeded.
   */
  bool save();

  /**
   * @brief Writes the current Key Registry to `/apikeys.json`.
   * @return True if save succeeded.
   */
  bool saveApiKeys();

  /**
   * @brief Adds or updates a key in the registry.
   */
  void updateKey(const ApiKey &key);

  /**
   * @brief Removes a key from the registry by ID.
   */
  void deleteKey(const char *id);

  /**
   * @brief Retrieves a key from the registry by ID.
   */
  ApiKey *getKeyById(const char *id);

  /**
   * @brief Save textual data string to a file in LittleFS.
   * @param fName Absolute path to the destination file.
   * @param fData Content string to write.
   * @return True if write succeeded.
   */
  bool saveFile(String fName, String fData);

  /**
   * @brief Load textual data from a LittleFS file into a String.
   * @param fName Absolute path to the source file.
   * @return Content of the file as a String.
   */
  String loadFile(String fName);

  /**
   * @brief Access the current configuration (Mutable).
   * @return Reference to the internal Config struct.
   */
  Config &getConfig() { return config; }

  /**
   * @brief Access the current configuration (Read-only).
   * @return Const reference to the internal Config struct.
   */
  const Config &getConfig() const { return config; }

  /**
   * @brief Checks if there is at least one fully configured board available.
   * @return True if at least one board's .complete flag is true.
   */
  bool hasConfiguredBoards() const {
    for (int i = 0; i < config.boardCount; i++) {
      if (config.boards[i].complete)
        return true;
    }
    return false;
  }

  /**
   * @brief Register a component to receive configuration updates.
   * @param consumer Pointer to a class implementing iConfigurable.
   */
  void registerConsumer(class iConfigurable *consumer);

  /**
   * @brief Notify all registered consumers to re-apply the current
   * configuration.
   */
  void notifyConsumersToReapplyConfig();

  bool reloadPending = false; // Flag to safely trigger reconfiguration defensively on the main thread

  /**
   * @brief Requests a pending reconfiguration by setting the flag to true.
   */
  void requestReload() { reloadPending = true; }

  /**
   * @brief Examines the pending reload flag and resets it if necessary.
   * @return True if a reload was pending and is now cleared, false otherwise.
   */
  bool checkAndClearReload() {
    if (reloadPending) {
      reloadPending = false;
      return true;
    }
    return false;
  }

private:
  static const int MAX_CONSUMERS =
      MAX_CONFIG_CONSUMERS; // Maximum number of configurable modules
  class iConfigurable
      *consumers[MAX_CONSUMERS]; // Array of registered consumer pointers
  int consumerCount = 0;         // Current number of registered consumers
};

// --- System-wide Orchestration ---
// Individual modules access configuration via appContext::getConfigManager()
