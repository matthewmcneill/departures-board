/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/configManager/configManager.hpp
 * Description: Manages the loading, saving, and parsing of application configurations 
 *              and API keys stored in JSON format on LittleFS.
 *
 * Exported Functions/Classes:
 * - BoardModes: Enum defining supported operational modes (Rail, Tube, Bus).
 * - Config: Struct containing all system and user application settings.
 * - ConfigManager: Class for handling configuration persistence and access.
 * - configManager: Global singleton for configuration management.
 */

#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <WiFi.h>

#include <buildOptions.h>

/**
 * @brief Supported operational types for the display boards.
 */
enum BoardTypes {
    MODE_RAIL = 0,
    MODE_TUBE = 1,
    MODE_BUS = 2
};

/**
 * @brief Configuration for a single display board instance.
 */
struct BoardConfig {
    BoardTypes type = MODE_RAIL; // Type of board (Rail, Tube, Bus)
    char name[80] = "";          // Human-readable name for the station/stop
    char id[13] = "";            // ID (CRS for Rail, Naptan for Tube, Atco for Bus)
    
    // --- Location Settings ---
    float lat = 0.0f;            // Latitude for weather/local info
    float lon = 0.0f;            // Longitude for weather/local info
    
    // --- Filters & Filters ---
    char filter[54] = "";        // Platform filter or Bus route whitelist
    char secondaryId[13] = "";   // Calling station CRS or secondary filter
    char secondaryName[45] = ""; // Human name for calling filter
    
    // --- Timing ---
    int timeOffset = 0;          // Local board time adjustment (NR specific)

    // --- Features ---
    bool showWeather = true;     // Enable/Disable weather overlay for this board

    // --- Runtime Readiness (Computed by ConfigManager) ---
    bool complete = false;       // True if all mandatory fields are present
    int errorType = 0;           // 0: OK, 1: Missing API Keys, 2: Missing Station ID
};

/**
 * @brief Container for all application configuration settings.
 */
struct Config {
    // --- API Keys & Tokens ---
    char nrToken[37] = ""; // National Rail Darwin token
    char tflAppkey[50] = ""; // TfL application key
    char owmToken[33] = ""; // OpenWeatherMap API key
    bool apiKeysLoaded = false; // Flag indicating if secrets are successfully loaded

    // --- System Settings ---
    char hostname[33] = "DeparturesBoard"; // Network hostname
    char wsdlHost[48] = "lite.realtime.nationalrail.co.uk"; // WSDL service host
    char wsdlAPI[48] = "/OpenLDBWS/wsdl.aspx?ver=2021-11-01"; // WSDL path
    char timezone[64] = "Europe/London"; // POSIX timezone string
    float configVersion = 2.1f; // Configuration format version number (v2.1 includes type/lat/lon standardization)
    
    // --- Display Preferences ---
    bool dateEnabled = false; // Show date in header
    bool showClockInSleep = true; // Show clock during snooze
    bool noScrolling = false; // Disable panel animations
    bool hidePlatform = false; // Suppress platform columns
    bool flipScreen = false; // Rotate display 180 degrees
    int brightness = 20; // Hardware brightness level (0-255)
    
    // --- Sleep Settings ---
    bool sleepEnabled = false; // Timer-based power management
    int sleepStarts = 23; // Hour to enter sleep (0-23)
    int sleepEnds = 8; // Hour to wake up (0-23)

    // --- Refresh & Update Settings ---
    long apiRefreshRate = DATAUPDATEINTERVAL; // Current polling interval
    bool firmwareUpdatesEnabled = false; // Enable background OTA checks
    bool dailyUpdateCheckEnabled = false; // Check for firmware updates at midnight

    // --- Dynamic Board List ---
    BoardConfig boards[MAX_BOARDS]; // Array of configured display boards
    int boardCount = 0;             // Number of active boards in the carousel
    int defaultBoardIndex = 0;      // Index of the board to show on startup

    // --- Legacy Stubs (Compatibility with Web UI) ---
    // These fields are synchronized with the boards array for backward compatibility
    BoardTypes boardType = MODE_RAIL; // Legacy: Maps to boards[0].type
    char crsCode[4] = "";            // Legacy: Maps to boards[0].id
    float stationLat = 0.0f;         // Legacy: Maps to boards[0].lat
    float stationLon = 0.0f;         // Legacy: Maps to boards[0].lon
    char callingCrsCode[4] = "";     // Legacy: Maps to boards[0].secondaryId
    char callingStation[45] = "";    // Legacy: Maps to boards[0].secondaryName
    char platformFilter[54] = "";    // Legacy: Maps to boards[0].filter
    int nrTimeOffset = 0;            // Legacy: Maps to boards[0].timeOffset

    bool altStationEnabled = false;  // Legacy: Managed via array presence
    char altCrsCode[4] = "";         // Legacy: Maps to an additional NR board
    float altLat = 0.0f;             // Legacy
    float altLon = 0.0f;             // Legacy
    char altCallingCrsCode[4] = "";  // Legacy
    char altCallingStation[45] = ""; // Legacy
    char altPlatformFilter[54] = ""; // Legacy
    int altStarts = 0;               // Legacy: Unsupported in new format (yet)
    int altEnds = 0;                 // Legacy: Unsupported in new format (yet)

    char tubeId[13] = "";            // Legacy: Maps to a TFL board in array
    char tubeName[80] = "";          // Legacy: Maps to a TFL board in array

    bool showBus = false;            // Legacy: Managed via array presence
    char busId[13] = "";             // Legacy: Maps to a BUS board in array
    char busName[80] = "";           // Legacy: Maps to a BUS board in array
    float busLat = 0.0f;             // Legacy
    float busLon = 0.0f;             // Legacy
    char busFilter[54] = "";         // Legacy

    char rssUrl[128] = ""; // XML feed URL
    char rssName[48] = ""; // Label for news source
    bool rssEnabled = false; // Enable headlines scroller
};

/**
 * @brief Class responsible for persistence and access of the Config object.
 */
class ConfigManager {
private:
    Config config; // Master configuration state container

public:
    ConfigManager() = default;

    /**
     * @brief Load all user preferences and module settings from `/config.json`.
     */
    void loadConfig();

    /**
     * @brief Load sensitive tokens (NR token, OWM token, TfL App Key) from `/apikeys.json`.
     */
    void loadApiKeys();

    /**
     * @brief Validates all configured boards and updates their 'complete' and 'errorType' properties.
     */
    void validate();

    /**
     * @brief Writes a skeleton `/config.json` configuration onto LittleFS.
     */
    void writeDefaultConfig();

    /**
     * @brief Writes the current in-memory configuration to `/config.json` in the modern multi-board format.
     * @return True if save succeeded.
     */
    bool save();

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
    Config& getConfig() { return config; }

    /**
     * @brief Access the current configuration (Read-only).
     * @return Const reference to the internal Config struct.
     */
    const Config& getConfig() const { return config; }

    /**
     * @brief Register a component to receive configuration updates.
     * @param consumer Pointer to a class implementing iConfigurable.
     */
    void registerConsumer(class iConfigurable* consumer);

    /**
     * @brief Notify all registered consumers to re-apply the current configuration.
     */
    void notifyConsumersToReapplyConfig();

private:
    static const int MAX_CONSUMERS = MAX_CONFIG_CONSUMERS; // Maximum number of configurable modules
    class iConfigurable* consumers[MAX_CONSUMERS]; // Array of registered consumer pointers
    int consumerCount = 0; // Current number of registered consumers
};

// --- System-wide Orchestration ---
extern ConfigManager configManager; // Global singleton for configuration handling
