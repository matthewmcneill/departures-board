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
#include <gfx/fonts.h>
#include <gfx/xbmgfx.h>
#include <webgui/webgraphics.h>
#include <webgui/index.h>
#include <webgui/keys.h>
#include <webgui/pages.h>
#include <webgui/rssfeeds.h>

// Internal Modules & Managers
#include <Logger.hpp>
#include <WiFiConfig.hpp>
#include <configManager.hpp>
#include <timeManager.hpp>
#include <displayManager.hpp>
#include <otaUpdater.hpp>
#include <webServer.hpp>
#include "../lib/boards/systemBoard/include/systemBoard.hpp"
#include "../lib/boards/interfaces/IStation.hpp"

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

// Display Instance using Hardware SPI
U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ DISPLAY_CS_PIN, /* dc=*/ DISPLAY_DC_PIN, /* reset=*/ U8X8_PIN_NONE);

// GitHub OTA Client
github ghUpdate("", "");

// Weather API Client
weatherClient* currentWeather = new weatherClient();

// RSS Feeds Client
rssClient* rss = new rssClient();

// Shared Message Pool for board displays
stnMessages messages;

// API Service Boards
NationalRailBoard nationalRailBoard;
TfLBoard* tflBoard = new TfLBoard();
BusBoard* busBoard = new BusBoard();


// -----------------------------------------------------------------------------
// Environment Variables & State Management 
// -----------------------------------------------------------------------------

// Versioning
int VERSION_MAJOR = 2;
int VERSION_MINOR = 2;

// Operating Mode (Rail/Tube/Bus)
boardModes boardMode = MODE_RAIL;

// API Credits and Attribution Text
extern const char nrAttributionn[] = "Powered by National Rail Enquiries";
extern const char tflAttribution[] = "Powered by TfL Open Data";
extern const char btAttribution[] = "Powered by bustimes.org";

// Hardware Display Configuration
bool hidePlatform = false;          // Hide platform numbers?
bool noScrolling = false;           // Mute horizontal scrolling messages

// Display Sleep Configurations
// Display Sleep Configurations
// Managed entirely by displayManager internally

// Display State
int dateWidth;                      // Pixel width of the rendered date text
int dateDay;                        // Integer day of month
bool dateEnabled = false;           // Enable date rendering
bool clockEnabled = true;           // Enable main clock rendering
char displayedTime[29] = "";        // Clock string buffer
unsigned long nextClockUpdate = 0;  // Millis until next clock check
int fpsDelay = 25;                  // Millis between matrix rendering steps

// Networking State
const char defaultHostname[] = "DeparturesBoard";
char hostname[33];                  // Explicit system hostname (e.g. mDNS)
char myUrl[24];                     // Formatted string URL for local IP

// API State Flags
bool apiKeys = false;               // Determines if external APIs are loaded
char wsdlHost[MAXHOSTSIZE];         // Cached endpoint host string
char wsdlAPI[MAXAPIURLSIZE];        // Cached endpoint URI string

// Polling and Refresh Management
unsigned long refreshTimer = 0;           // Timer cache
unsigned long nextDataUpdate = 0;         // Calculated time to hit main data endpoints again
long apiRefreshRate = DATAUPDATEINTERVAL; // Dynamically calculated endpoint polling wait time
bool noDataLoaded = true;                 // Trigger if payload array counts are zero
int dataLoadSuccess = 0;                  // Incremental tracker of valid JSON parses
int dataLoadFailure = 0;                  // Incremental tracker of connection breaks
unsigned long lastLoadFailure = 0;        // Last exception time
unsigned long lastDataLoadTime = 0;       // Timestamp of last data load
int lastUpdateResult = 0;                 // HTTP return codes

// Tube & Bus API Stores
// Extracted to TfLBoard and BusBoard



// -----------------------------------------------------------------------------
// Boot Setup
// -----------------------------------------------------------------------------

/**
 * @brief setup
 * Main execution boot block. Wakes the OLED, formats filesystem partitions, starts the 
 * networking and webserver background threads, initializes API credentials, provisions
 * the active Display Board, checks for firmware updates, hooks the NTP clock offset, 
 * pre-fetches any addons, and prepares the main memory cache strings.
 */
void setup(void) {
  // Default WSDL endpoints
  strncpy(wsdlHost, "lite.realtime.nationalrail.co.uk", sizeof(wsdlHost));
  strncpy(wsdlAPI, "/OpenLDBWS/wsdl.aspx?ver=2021-11-01", sizeof(wsdlAPI));

  // Initialize Matrix Display Output
  u8g2.begin();
  u8g2.setContrast(displayManager.getBrightness());
  u8g2.setDrawColor(1);
  u8g2.setFontMode(1);
  u8g2.setFontRefHeightAll();
  u8g2.setFontPosTop();
  u8g2.setFont(NatRailTall12);

  String buildDate = String(__DATE__);
  String notice = "\x80 " + buildDate.substring(buildDate.length()-4) + F(" Gadec Software (github.com/gadec-uk)");

  // Initialize Data Partition
  LOG_INFO("Mounting LittleFS...");
  bool isFSMounted = LittleFS.begin(true);
  if (!isFSMounted) LOG_WARN("LittleFS failed to mount. Format may be required.");
  else LOG_INFO("LittleFS mounted successfully.");

  // Inject active board struct
  if (boardMode == MODE_RAIL) {
      displayManager.setBoardType(0, BoardType::NR_BOARD);
  } else if (boardMode == MODE_TUBE) {
      displayManager.setBoardType(0, BoardType::TFL_BOARD);
  } else if (boardMode == MODE_BUS) {
      displayManager.setBoardType(0, BoardType::BUS_BOARD);
  }

  // Inject API Configuration Data
  currentWeather->setWeatherMsg("");
  nationalRailBoard.setNrToken("");
  tflBoard->setTflAppkey("");
  loadApiKeys();
  
#ifdef ENABLE_DEBUG_LOG
  Serial.begin(115200);
  delay(2000); // Give the serial monitor time to connect after a flash/reset
#endif
  
  Logger::registerSecret(String(nationalRailBoard.getNrToken()));
  Logger::registerSecret(String(tflBoard->getTflAppkey()));

  LOG_INFO("Starting Departures Board...");
  
  LOG_INFO("Loading configuration settings...");
  loadConfig();
  u8g2.setContrast(displayManager.getBrightness());
  if (displayManager.getFlipScreen()) u8g2.setFlipMode(1);

  // Initial Boot Splash Render
  u8g2.clearBuffer();
  u8g2.drawXBM(81, 0, gadeclogo_width, gadeclogo_height, gadeclogo_bits);
  centreText(notice.c_str(), 48);
  u8g2.sendBuffer();
  delay(5000);

  u8g2.clearBuffer();
  drawStartupHeading();
  u8g2.sendBuffer();
  progressBar(F("Connecting to Wi-Fi"), 20);

  // Network Provisioning Phase
  setupWiFi(hostname, wmConfigModeCallback);
  updateMyUrl();
  
  if (MDNS.begin(hostname)) {
    MDNS.addService("http", "tcp", 80);
    LOG_INFO(String("mDNS responder started: ") + hostname + ".local");
  }

  LOG_INFO("WiFi Connected successfully.");
  setWifiConnected(true);
  WiFi.setAutoReconnect(true);

  u8g2.clearBuffer();
  drawStartupHeading();
  char ipBuff[17];
  WiFi.localIP().toString().toCharArray(ipBuff, sizeof(ipBuff));
  centreText(ipBuff, 53);
  progressBar(F("Wi-Fi Connected"), 30);
  u8g2.sendBuffer();

  // Load Administrative Background Threads
  webServer.init();
  // Check if OTA Updates Should Happen On Boot
  if (ota.getFirmwareUpdatesEnabled()) {
    LOG_INFO("Checking for firmware updates on GitHub...");
    progressBar(F("Checking for firmware updates"), 40);
    if (ghUpdate.getLatestRelease()) {
      LOG_INFO("Firmware updates found. Attempting update...");
      ota.checkForFirmwareUpdate();
    } else {
      char errLog[128];
      snprintf(errLog, sizeof(errLog), "Firmware update check failed: %s", ghUpdate.getLastError());
      LOG_WARN(errLog);
      for (int i=15; i>=0; i--) {
        showFirmwareUpdateCompleteScreen("Firmware Update Check Failed", "Unable to retrieve latest release information.", ghUpdate.getLastError(), "", i, false);
        delay(1000);
      }
      u8g2.clearDisplay();
      drawStartupHeading();
      u8g2.sendBuffer();
    }
  }
  checkPostWebUpgrade();

  // Manual Handshake Web Check
  if ((!nationalRailBoard.getCrsCode()[0] && !tflBoard->getTubeId()[0] && !busBoard->getBusAtco()[0]) || (!nationalRailBoard.getNrToken()[0] && boardMode == MODE_RAIL)) {
    if (!apiKeys) showSetupKeysHelpScreen();
    else showSetupCrsHelpScreen();
    // Yield hardware to Web GUI Setup if there's no stored Config
    while (true) {
      yield();
      webServer.handleClient();
    }
  }

  // System Time Initialization
  if (!timeManager.initialize()) {
    progressBar(F("Failed to set the clock. Rebooting in 5 sec."), 0);
    delay(5000);
    ESP.restart();
  }

  // Addon Loading
  if (rss->getRssEnabled() && boardMode != MODE_BUS) {
    progressBar(F("Loading RSS headlines feed"), 60);
    updateRssFeed();
  }

  if (currentWeather->getWeatherEnabled() && boardMode != MODE_TUBE) {
    progressBar(F("Getting weather conditions"), 64);
    updateCurrentWeather(nationalRailBoard.getStationLat(), nationalRailBoard.getStationLon());
  }

  // Launching
  if (boardMode == MODE_RAIL) {
      LOG_INFO("Initialising National Rail Interface (board mode)...");
      progressBar(F("Initialising National Rail interface"), 70);
      nationalRailBoard.setAltStationActive(setAlternateStation());
  } else if (boardMode == MODE_TUBE) {
      LOG_INFO("Initialising TfL Interface (board mode)...");
      progressBar(F("Initialising TfL interface"), 70);
      setStartupProgressPercent(70);
  } else if (boardMode == MODE_BUS) {
      LOG_INFO("Initialising BusTimes Interface (board mode)...");
      progressBar(F("Initialising BusTimes interface"), 70);
      setStartupProgressPercent(70);
  }
}

// -----------------------------------------------------------------------------
// Core Execution Loop
// -----------------------------------------------------------------------------

/**
 * @brief loop
 * Endlessly loops polling API caches asynchronously according to specified limits.
 * Triggers hardware renders by calling down the abstract display manager. Checks
 * OTA thread hooks and WebServer client requests continuously.
 */
void loop(void) {
  // Firmware Background Checks
  ota.tick();

  // Matrix Draw Dispatcher
  displayManager.tick(millis());

  // Connection Management Graphics
  if (WiFi.status() != WL_CONNECTED && getWifiConnected()) {
    setWifiConnected(false);
    u8g2.setFont(NatRailSmall9);
    u8g2.drawStr(0, 56, "\x7F");
    u8g2.updateDisplayArea(0, 7, 1, 1);
  } else if (WiFi.status() == WL_CONNECTED && !getWifiConnected()) {
    setWifiConnected(true);
    blankArea(0, 57, 5, 7);
    u8g2.updateDisplayArea(0, 7, 1, 1);
    updateMyUrl();  // Refresh dynamic IP layout
  }

  if (WiFi.status() != WL_CONNECTED && millis() > getLastWiFiReconnect() + 10000) {
    WiFi.disconnect();
    delay(100);
    WiFi.reconnect();
    setLastWiFiReconnect(millis());
  }

  // Active Screen Loop Polling
  if (!displayManager.getIsSleeping()) {
    if (millis() > nextDataUpdate && getWifiConnected()) {
      showFirmwareUpdateIcon(true);
      lastUpdateResult = displayManager.getActiveBoard()->updateData();
      
      // Select next update interval
      if (boardMode == MODE_RAIL) nextDataUpdate = millis() + apiRefreshRate;
      else if (boardMode == MODE_TUBE) nextDataUpdate = millis() + UGDATAUPDATEINTERVAL;
      else nextDataUpdate = millis() + BUSDATAUPDATEINTERVAL;

      // Handle Result Codes
      if (lastUpdateResult == UPD_SUCCESS || lastUpdateResult == UPD_NO_CHANGE) {
        showFirmwareUpdateIcon(false);
        lastDataLoadTime = millis();
        noDataLoaded = false;
        dataLoadSuccess++;
        
        // Asynchronously update weather and RSS where able
      if (currentWeather->getWeatherEnabled() && boardMode != MODE_TUBE && millis() > currentWeather->getNextWeatherUpdate()) updateCurrentWeather(nationalRailBoard.getStationLat(), nationalRailBoard.getStationLon());
      if (rss->getRssEnabled() && boardMode != MODE_BUS && millis() > rss->getNextRssUpdate()) updateRssFeed();
        
        // Final Screen Flush
        u8g2.clearBuffer();
        displayManager.getActiveBoard()->render(u8g2);
      } else if (lastUpdateResult == UPD_UNAUTHORISED) {
        showTokenErrorScreen();
      } else {
        lastLoadFailure = millis();
        dataLoadFailure++;
        nextDataUpdate = millis() + 30000;
        showFirmwareUpdateIcon(false);
        if (noDataLoaded) showNoDataScreen();
      }
    }

    // Ignore time delays for Initial Fetch
    if (getFirstLoad()) {
        setFirstLoad(false);
        u8g2.clearBuffer();
        displayManager.getActiveBoard()->render(u8g2);
    }
  }

  // Thread Yield for Local HTTP Server Client Listeners
  webServer.handleClient();
}
