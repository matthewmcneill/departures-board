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
 * Provides:
 * - saveFile() / loadFile(): File system utilities for strings.
 * - checkPostWebUpgrade(): Cleans up legacy artifacts after a web UI update.
 * - loadApiKeys() / loadConfig() / writeDefaultConfig(): Core config lifecycle.
 * - Extern declarations for legacy global state variables.
 */

#pragma once


#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <WiFi.h>

extern const char defaultHostname[];

extern char nrToken[37];
extern String tflAppkey;
extern bool apiKeys;

extern char crsCode[4];
extern char callingCrsCode[4];
extern char callingStation[45];
extern char platformFilter[54];
extern char hostname[33];
extern char myUrl[24];
extern char wsdlHost[48];
extern char wsdlAPI[48];

extern bool dateEnabled;
extern bool enableBus;
extern long apiRefreshRate;

extern float stationLat;
extern float stationLon;

enum boardModes {
  MODE_RAIL = 0,
  MODE_TUBE = 1,
  MODE_BUS = 2
};
extern boardModes boardMode;

extern char altCrsCode[4];
extern bool altStationEnabled;
extern byte altStarts;
extern byte altEnds;
extern float altLat;
extern float altLon;
extern char altCallingCrsCode[4];
extern char altCallingStation[45];
extern char altPlatformFilter[54];

extern bool noScrolling;
extern int nrTimeOffset;
extern bool hidePlatform;

extern bool altStationActive;

#ifndef DATAUPDATEINTERVAL
#define DATAUPDATEINTERVAL 150000
#endif
#ifndef FASTDATAUPDATEINTERVAL
#define FASTDATAUPDATEINTERVAL 45000
#endif

// Config management functions

/**
 * @brief Save textual data string to a file in LittleFS.
 * @param fName Absolute path to the file.
 * @param fData Data string to save.
 * @return True if the file was opened and written successfully, otherwise false.
 */
bool saveFile(String fName, String fData);

/**
 * @brief Load textual data from a LittleFS file into a String.
 * @param fName Absolute path to the file.
 * @return File contents as a String, or empty string on failure.
 */
String loadFile(String fName);

/**
 * @brief Compare current web interface version against written states, deleting obsolete cached static assets.
 */
void checkPostWebUpgrade();

/**
 * @brief Load sensitive tokens (NR token, OWM token, TfL App Key) from `/apikeys.json`.
 */
void loadApiKeys();

/**
 * @brief Writes a skeleton `/config.json` configuration onto LittleFS.
 */
void writeDefaultConfig();

/**
 * @brief Load all user preferences and module settings from `/config.json` and inject them into objects.
 */
void loadConfig();
