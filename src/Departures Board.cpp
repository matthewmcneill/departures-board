/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Supported Hardware:
 * - ESP32 "Mini" (env: esp32dev)
 * - Waveshare ESP32-S3-Nano (env: esp32s3nano)
 * Both use a 3.12" 256x64 OLED Display Panel with SSD1322 controller on-board.
 *
 * PIN CONNECTIONS (Configurable via platformio.ini):
 * OLED PANEL     ESP32 MINI     ESP32-S3 NANO
 * 1  VSS         GND            GND
 * 2  VCC_IN      3.3V           3.3V
 * 4  D0/CLK      IO18           D13 (SCK)
 * 5  D1/DIN      IO23           D11 (COPI/MOSI)
 * 14 D/C#        DISPLAY_DC_PIN DISPLAY_DC_PIN  (e.g., IO5, D9)
 * 16 CS#         DISPLAY_CS_PIN DISPLAY_CS_PIN  (e.g., IO26, D10)
 *
 * NOTE ON HARDWARE SPI:
 * Because we are using the U8G2_..._HW_SPI (Hardware SPI) version of the display 
 * library rather than a Software SPI setup, the U8g2 library intentionally doesn't ask 
 * for the Clock and Data pin numbers simply because those pins are physically hardwired
 * inside the ESP32 chip to dedicated hardware micro-controllers for maximum performance.
 *
 * When we tell PlatformIO to build for board = arduino_nano_esp32 or board = esp32dev, 
 * it pulls in the specific Arduino Core variants for those boards under the hood. 
 * The core automatically maps the default Hardware SPI pathways to the correct pins 
 * for that respective board without us having to lift a finger in our codebase!
 *
 * Module: src/Departures Board.cpp
 * Description: Main application entry point for the Departures Board.
 *
 * Exported Functions/Classes:
 * - setup: Hardware and software initialization.
 * - loop: Main application polling and animation loop.
 */

// -----------------------------------------------------------------------------
// Libraries and Includes
// -----------------------------------------------------------------------------

// Core Library Includes
#include <Arduino.h>
#include <SPI.h>
#include <FS.h>
#include <LittleFS.h>
#include <time.h>

// Parse libraries
#include <JsonListener.h>
#include <JsonStreamingParser.h>

// Networking & Web Libraries
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <ESPmDNS.h>
#include <WiFiManager.h>

// Utilities
#include <ArduinoJson.h>

// Third-Party Firmware Management
#include <HTTPUpdateGitHub.h>

// UI, Font, and Display Drivers
#include <U8g2lib.h>
#include <fonts/fonts.hpp>
#include <gfx/xbmgfx.h>
#include <webgui/webgraphics.h>
#include <webgui/pages.h>
#include <webgui/rssfeeds.h>

#include <widgets/wifiStatusWidget.hpp>
#include <widgets/imageWidget.hpp>

// Internal Modules & Managers
#include <Logger.hpp>
#include <WiFiConfig.hpp>
#include <configManager.hpp>
#include <timeManager.hpp>
#include <boards/systemBoard/systemBoard.hpp>
#include <boards/systemBoard/loadingBoard.hpp>
#include <boards/systemBoard/firmwareUpdateBoard.hpp>
#include <boards/systemBoard/splashBoard.hpp>
#include <boards/interfaces/iDisplayBoard.hpp>
#include <displayManager.hpp>
#include <otaUpdater.hpp>
#include <webServer.hpp>

// API Service Clients
#include <weatherClient.h>
#include <githubClient.h>
#include <rssClient.h>


// -----------------------------------------------------------------------------
// Definitions & Macros
// -----------------------------------------------------------------------------

// Timers
#define msDay 86400000                 // 86400000 milliseconds in a day
#define msHour 3600000                 // 3600000 milliseconds in an hour
#define msMin 60000                    // 60000 milliseconds in a minute

#define SCREENSAVERINTERVAL 10000      // How often the screen is changed in sleep mode (ms)
#define DATAUPDATEINTERVAL 150000      // Default fetch interval from National Rail (ms)
#define FASTDATAUPDATEINTERVAL 45000   // Fast fetch interval from National Rail (ms)
#define UGDATAUPDATEINTERVAL 30000     // Fetch interval from TfL (ms)
#define BUSDATAUPDATEINTERVAL 45000    // Fetch interval from bustimes.org (ms)

// API Endpoint Constraints
#define MAXHOSTSIZE 48                 // Maximum size of the wsdl Host
#define MAXAPIURLSIZE 48               // Maximum size of the wsdl url

// OLED Display Matrix Row Offsets (National Rail)
#define LINE0 0
#define LINE1 13
#define LINE2 28
#define LINE3 41
#define LINE4 55

// OLED Display Matrix Row Offsets (Underground)
#define ULINE0 0
#define ULINE1 15
#define ULINE2 28
#define ULINE3 41
#define ULINE4 56

// -----------------------------------------------------------------------------
// Global Object Instantiations
// -----------------------------------------------------------------------------

// Display Instance is now encapsulated within DisplayManager

// Configuration Manager
ConfigManager configManager;

// GitHub OTA Client
github ghUpdate("", "");

// Weather API Client
weatherClient* currentWeather = new weatherClient();

// RSS Feeds Client
rssClient* rss = new rssClient();

// Shared Message Pool for board displays
stnMessages messages;

// API Service Boards (Managed via DisplayManager carousel)

// -----------------------------------------------------------------------------
// Environment Variables & State Management 
// -----------------------------------------------------------------------------

// Versioning
int VERSION_MAJOR = 2;
int VERSION_MINOR = 2;

// API Credits and Attribution Text
extern const char nrAttributionn[] = "Powered by National Rail Enquiries";
extern const char tflAttribution[] = "Powered by TfL Open Data";
extern const char btAttribution[] = "Powered by bustimes.org";

// Display State (Managed by displayManager, but kept for legacy logic if needed)
char displayedTime[29] = "";        // Clock string buffer
unsigned long nextClockUpdate = 0;  // Millis until next clock check

// Networking State
char myUrl[24];                     // Formatted string URL for local IP

// Polling and Refresh Management
unsigned long nextDataUpdate = 0;         // Calculated time to hit main data endpoints again
bool noDataLoaded = true;                 // Trigger if payload array counts are zero
int dataLoadSuccess = 0;                  // Incremental tracker of valid JSON parses
int dataLoadFailure = 0;                  // Incremental tracker of connection breaks
unsigned long lastLoadFailure = 0;        // Last exception time (ms)
unsigned long lastDataLoadTime = 0;       // Timestamp of last data load (ms)
int lastUpdateResult = 0;                 // HTTP return codes from last poll

// Tube & Bus API Stores
// Extracted to TfLBoard and BusBoard



// -----------------------------------------------------------------------------
// Boot Setup
// -----------------------------------------------------------------------------

/**
 * @brief Initialize hardware and software components.
 */
void setup(void) {
  #ifdef ENABLE_DEBUG_LOG
    Serial.begin(115200);
    delay(2000); // Give the serial monitor time to connect after a flash/reset
  #endif
  
  // --- Step 1: Hardware & Storage Initialization ---
  displayManager.begin();

  String buildDate = String(__DATE__);
  String notice = "\x80 " + buildDate.substring(buildDate.length()-4) + F(" Gadec Software (github.com/gadec-uk)");

  // Initialize Data Partition
  LOG_INFO("Mounting LittleFS...");
  bool isFSMounted = LittleFS.begin(true);
  if (!isFSMounted) LOG_WARN("LittleFS failed to mount. Format may be required.");
  else LOG_INFO("LittleFS mounted successfully.");

  // --- Step 2: System Configuration Phase ---
  currentWeather->setWeatherMsg("");

  configManager.loadApiKeys();
  const Config& config = configManager.getConfig();
  Logger::registerSecret(String(config.nrToken));
  Logger::registerSecret(String(config.tflAppkey));
  //TODO there should be another ky here for the bus times

  LOG_INFO("Starting Departures Board...");
  
  LOG_INFO("Loading configuration settings...");
  configManager.loadConfig();

  // --- Step 3: Register Configuration Consumers ---
  configManager.registerConsumer(&displayManager);
  configManager.registerConsumer(currentWeather);
  configManager.registerConsumer(rss);
  configManager.registerConsumer(&timeManager);
  configManager.registerConsumer(&wifiManager);
  configManager.registerConsumer(&ota);

  // Initial distribution of configuration
  configManager.notifyConsumersToReapplyConfig();

  // --- Step 4: Boot UI and Animation ---
  SplashBoard* splash = (SplashBoard*)displayManager.getSystemBoard(SystemBoardId::SYS_BOOT_SPLASH);
  splash->setNotice(notice.c_str());
  displayManager.showBoard(splash, 5000);

  LoadingBoard* load = (LoadingBoard*)displayManager.getSystemBoard(SystemBoardId::SYS_BOOT_LOADING);
  load->setProgress("Connecting to Wi-Fi", 20);
  displayManager.showBoard(load);

  // Network Provisioning Phase
  wifiManager.begin(config.hostname, wmConfigModeCallback);
  updateMyUrl();
  
  if (MDNS.begin(config.hostname)) {
    MDNS.addService("http", "tcp", 80);
    LOG_INFO(String("mDNS responder started: ") + config.hostname + ".local");
  }

  LOG_INFO("WiFi Connected successfully.");
  setWifiConnected(true);
  WiFi.setAutoReconnect(true);

  char ipBuff[17];
  WiFi.localIP().toString().toCharArray(ipBuff, sizeof(ipBuff));
  load->setProgress("Wi-Fi Connected", 30);
  load->setNotice(ipBuff); // IP flash overlay handled via notice field
  displayManager.showBoard(load);

  // Load Administrative Background Threads
  webServer.init();
  // Check if OTA Updates Should Happen On Boot
  if (config.firmwareUpdatesEnabled && getWifiConnected()) {
    LOG_INFO("Checking for firmware updates on GitHub...");
    load->setProgress("Checking for firmware updates", 40);
    displayManager.showBoard(load);
    if (ghUpdate.getLatestRelease()) {
      LOG_INFO("Firmware updates found. Attempting update...");
      ota.checkForFirmwareUpdate();
    }
    displayManager.showBoard(load);  // incase the ota loaded the wizard board
  }
  ota.checkPostWebUpgrade();

  // Manual Handshake Web Check
  if ((!config.crsCode[0] && !config.tubeId[0] && !config.busId[0]) || (!config.nrToken[0] && config.boardMode == MODE_RAIL)) {
    if (!config.apiKeysLoaded) displayManager.showBoard(displayManager.getSystemBoard(SystemBoardId::SYS_HELP_KEYS));
    else displayManager.showBoard(displayManager.getSystemBoard(SystemBoardId::SYS_HELP_CRS));
    // Yield hardware to Web GUI Setup if there's no stored Config
    while (true) {
      yield();
      webServer.handleClient();
    }
  }

  if (!timeManager.initialize()) {
    load->setProgress("Failed to set the clock. Rebooting in 5 sec.", 0);
    displayManager.showBoard(load);
    delay(5000);
    ESP.restart();
  }

  // Addon Loading
  if (config.rssEnabled && config.boardMode != MODE_BUS) {
    load->setProgress("Loading RSS headlines feed", 60);
    displayManager.showBoard(load);
    updateRssFeed();
  }

  if (currentWeather->getWeatherEnabled() && config.boardMode != MODE_TUBE) {
    load->setProgress("Getting weather conditions", 64);
    displayManager.showBoard(load);
    updateCurrentWeather(config.stationLat, config.stationLon);
  }

  // Launching
  displayManager.showBoard(displayManager.getDisplayBoard(0));
  if (config.boardMode == MODE_RAIL) {
      LOG_INFO("Initialising National Rail Interface (board mode)...");
  } else if (config.boardMode == MODE_TUBE) {
      LOG_INFO("Initialising TfL Interface (board mode)...");
  } else if (config.boardMode == MODE_BUS) {
      LOG_INFO("Initialising BusTimes Interface (board mode)...");
  }
}

// -----------------------------------------------------------------------------
// Core Execution Loop
// -----------------------------------------------------------------------------

/**
 * @brief Executive rendering and state management loop.
 *        Asynchronously polls API caches, updates display via DisplayManager,
 *        and handles background tasks like OTA and WebServer.
 */
void loop(void) {
  // --- Step 1: Background Threads and UI Tick ---
  ota.tick();

  // Matrix Draw Dispatcher
  displayManager.tick(millis());

  // Connection Management Graphics
  if (WiFi.status() != WL_CONNECTED && getWifiConnected()) {
    setWifiConnected(false);
  } else if (WiFi.status() == WL_CONNECTED && !getWifiConnected()) {
    setWifiConnected(true);
    updateMyUrl();  // Refresh dynamic IP layout
  }

  if (WiFi.status() != WL_CONNECTED && millis() > getLastWiFiReconnect() + 10000) {
    WiFi.disconnect();
    delay(100);
    WiFi.reconnect();
    setLastWiFiReconnect(millis());
  }

  const Config& config = configManager.getConfig();

  // Active Screen Loop Polling
  static unsigned long lastDebugPrint = 0;
  if (millis() - lastDebugPrint > 5000) {
      char dbgMsg[128];
      snprintf(dbgMsg, sizeof(dbgMsg), "Loop tick. isSleeping: %d, millis: %lu, nextDataUpdate: %lu, WiFi: %d", displayManager.getIsSleeping(), millis(), nextDataUpdate, getWifiConnected());
      LOG_DEBUG(dbgMsg);
      lastDebugPrint = millis();
  }

  // --- Step 2: Active Screen Data Fetching ---
  if (!displayManager.getIsSleeping()) {
    if (millis() > nextDataUpdate && getWifiConnected()) {
      displayManager.setOtaUpdateAvailable(true);
      LOG_INFO("Triggering active board data update...");
      iDisplayBoard* activeBoard = displayManager.getDisplayBoard(displayManager.getActiveSlotIndex());
      lastUpdateResult = activeBoard->updateData();
      
      // Select next update interval
      if (config.boardMode == MODE_RAIL) nextDataUpdate = millis() + config.apiRefreshRate;
      else if (config.boardMode == MODE_TUBE) nextDataUpdate = millis() + UGDATAUPDATEINTERVAL;
      else nextDataUpdate = millis() + BUSDATAUPDATEINTERVAL;

      // Handle Result Codes
      if (lastUpdateResult == UPD_SUCCESS || lastUpdateResult == UPD_NO_CHANGE) {
        displayManager.setOtaUpdateAvailable(false);
        lastDataLoadTime = millis();
        noDataLoaded = false;
        dataLoadSuccess++;
        
        // Asynchronously update weather and RSS where able
        if (currentWeather->getWeatherEnabled() && config.boardMode != MODE_TUBE && millis() > currentWeather->getNextWeatherUpdate()) {
            float lat = 0, lon = 0;
            if (config.boardMode == MODE_RAIL) {
                lat = config.stationLat;
                lon = config.stationLon;
            } else if (config.boardMode == MODE_BUS) {
                lat = config.busLat;
                lon = config.busLon;
            }
            updateCurrentWeather(lat, lon);
        }
        if (rss->getRssEnabled() && config.boardMode != MODE_BUS && millis() > rss->getNextRssUpdate()) updateRssFeed();
        
        // Final Screen Flush
        displayManager.render();
      } else if (lastUpdateResult == UPD_UNAUTHORISED) {
        displayManager.showBoard(displayManager.getSystemBoard(SystemBoardId::SYS_ERROR_TOKEN));
      } else {
        lastLoadFailure = millis();
        dataLoadFailure++;
        nextDataUpdate = millis() + 30000;
        displayManager.setOtaUpdateAvailable(false);
        if (noDataLoaded) displayManager.showBoard(displayManager.getSystemBoard(SystemBoardId::SYS_ERROR_NO_DATA));
      }
    }

    // Ignore time delays for Initial Fetch
    if (getFirstLoad()) {
        setFirstLoad(false);
        displayManager.render();
    }
  }

  // --- Step 3: Handle External Requests ---
  // Thread Yield for Local HTTP Server Client Listeners
  webServer.handleClient();
}
