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

#ifndef DATAUPDATEINTERVAL
#define DATAUPDATEINTERVAL 150000 // Default data refresh interval in milliseconds
#endif
#ifndef FASTDATAUPDATEINTERVAL
#define FASTDATAUPDATEINTERVAL 45000 // Accelerated data refresh interval
#endif

/**
 * @brief Supported operational modes for the departures board.
 */
enum BoardModes {
  MODE_RAIL = 0, // National Rail LDBWS mode
  MODE_TUBE = 1, // London Underground TfL mode
  MODE_BUS = 2   // Bus Times mode
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

    // --- Operating Mode ---
    BoardModes boardMode = MODE_RAIL; // Current platform mode

    // --- National Rail Station (Main) ---
    char crsCode[4] = ""; // 3-letter station CRS
    float stationLat = 0.0f; // Latitude for local weather
    float stationLon = 0.0f; // Longitude for local weather
    int nrTimeOffset = 0; // Local board time adjustment
    char callingCrsCode[4] = ""; // CRS for calling station filter
    char callingStation[45] = ""; // Human name for calling filter
    char platformFilter[54] = ""; // Specific platform filter

    // --- National Rail Station (Alternative) ---
    bool altStationEnabled = false; // Enable timed station switching
    char altCrsCode[4] = ""; // Alt station CRS
    float altLat = 0.0f; // Alt station latitude
    float altLon = 0.0f; // Alt station longitude
    int altStarts = 0; // Hour to switch to Alt
    int altEnds = 0; // Hour to switch to Main
    char altCallingCrsCode[4] = ""; // Alt calling CRS filter
    char altCallingStation[45] = ""; // Alt calling human name
    char altPlatformFilter[54] = ""; // Alt platform filter

    // --- TfL / Tube Settings ---
    char tubeId[13] = ""; // Naptan ID for tube stop
    char tubeName[80] = ""; // Human name for tube stop

    // --- Bus Settings ---
    bool showBus = false; // Enable bus departures panel
    char busId[13] = ""; // ATCO code for bus stop
    char busName[80] = ""; // Human name for bus stop
    float busLat = 0.0f; // Bus stop latitude
    float busLon = 0.0f; // Bus stop longitude
    char busFilter[54] = ""; // Bus route whitelist

    // --- RSS Settings ---
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
     * @brief Writes a skeleton `/config.json` configuration onto LittleFS.
     */
    void writeDefaultConfig();

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
    static const int MAX_CONSUMERS = 10; // Maximum number of configurable modules
    class iConfigurable* consumers[MAX_CONSUMERS]; // Array of registered consumer pointers
    int consumerCount = 0; // Current number of registered consumers
};

// --- System-wide Orchestration ---
extern ConfigManager configManager; // Global singleton for configuration handling
