/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * ESP32 "Mini" Board with 3.12" 256x64 OLED Display Panel with SSD1322 controller on-board.
 *
 * OLED PANEL     ESP32 MINI
 * 1 VSS          GND
 * 2 VCC_IN       3.3V
 * 4 D0/CLK       IO18
 * 5 D1/DIN       IO23
 * 14 D/C#        IO5
 * 16 CS#         IO26
 *
 *
 * Module: src/Departures Board.cpp
 * Description: Exported functions and classes.
 *
 * Exported Functions/Classes:
 * - server: Server
 * - u8g2: U8g2
 * - ghUpdate: Gh update
 * - setup: Setup
 * - showSetupCrsHelpScreen: Show setup crs help screen
 * - loop: Loop
 */

// Release version number
#define VERSION_MAJOR 2
#define VERSION_MINOR 2

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <ESPmDNS.h>
#include <HTTPUpdateGitHub.h>
#include <FS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>
#include <weatherClient.h>
#include <stationData.h>
#include <raildataXmlClient.h>
#include <TfLdataClient.h>
#include <busDataClient.h>
#include <githubClient.h>
#include <rssClient.h>
#include <webgui/webgraphics.h>
#include <webgui/index.h>
#include <webgui/keys.h>
#include <webgui/pages.h>
#include <webgui/rssfeeds.h>
#include <gfx/fonts.h>
#include <gfx/xbmgfx.h>
#include <time.h>

#include <SPI.h>
#include <U8g2lib.h>

#define msDay 86400000 // 86400000 milliseconds in a day
#define msHour 3600000 // 3600000 milliseconds in an hour
#define msMin 60000 // 60000 milliseconds in a second

/**
 * @brief Server
 * @param 80
 * @return Return value
 */
WebServer server(80);     // Hosting the Web GUI
File fsUploadFile;        // File uploads

// Shorthand for response formats
static const char contentTypeJson[] PROGMEM = "application/json";
static const char contentTypeText[] PROGMEM = "text/plain";
static const char contentTypeHtml[] PROGMEM = "text/html";

// Using NTP to set and maintain the clock
static const char ntpServer[] PROGMEM = "europe.pool.ntp.org";
static struct tm timeinfo;
static const char ukTimezone[] = "GMT0BST,M3.5.0/1,M10.5.0";

// Default hostname
static const char defaultHostname[] = "DeparturesBoard";


#define SCREEN_WIDTH 256 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define DIMMED_BRIGHTNESS 1 // OLED display brightness level when in sleep/screensaver mode

// S3 Nano Hardware SPI (Clock=D13, Data=D11, CS=D10, DC=D9)
U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ U8X8_PIN_NONE);

// Vertical line positions on the OLED display (National Rail)
#define LINE0 0
#define LINE1 13
#define LINE2 28
#define LINE3 41
#define LINE4 55

// Vertical line positions on the OLED display (Underground)
#define ULINE0 0
#define ULINE1 15
#define ULINE2 28
#define ULINE3 41
#define ULINE4 56

// Service attribution texts
const char nrAttributionn[] = "Powered by National Rail Enquiries";
const char tflAttribution[] = "Powered by TfL Open Data";
const char btAttribution[] = "Powered by bustimes.org";

//
// GitHub Client for firmware updates
//  - Pass a GitHub token if updates are to be loaded from a private repository
//
github ghUpdate("","");

#define SCREENSAVERINTERVAL 10000     // How often the screen is changed in sleep mode (ms - 10 seconds)
#define DATAUPDATEINTERVAL 150000     // How often we fetch data from National Rail (ms - 2.5 mins) - "default" option
#define FASTDATAUPDATEINTERVAL 45000  // How often we fetch data from National Rail (ms - 45 secs) - "fast" option
#define UGDATAUPDATEINTERVAL 30000    // How often we fetch data from TfL (ms - 30 secs)
#define BUSDATAUPDATEINTERVAL 45000   // How often we fetch data from bustimes.org (ms - 45 secs)

// Bit and bobs
unsigned long timer = 0;
bool isSleeping = false;            // Is the screen sleeping (showing the "screensaver")
bool sleepEnabled = false;          // Is overnight sleep enabled?
bool forcedSleep = false;           // Is the system in manual sleep mode?
bool sleepClock = true;             // Showing the clock in sleep mode?
bool dateEnabled = false;           // Showing the date on screen?
bool weatherEnabled = false;        // Showing weather at station location. Requires an OpenWeatherMap API key.
bool enableBus = false;             // Include Bus services on the board?
bool firmwareUpdates = true;        // Check for and install firmware updates automatically at boot?
bool dailyUpdateCheck = false;      // Check for and install firmware updates at midnight?
byte sleepStarts = 0;               // Hour at which the overnight sleep (screensaver) begins
byte sleepEnds = 6;                 // Hour at which the overnight sleep (screensaver) ends
int brightness = 50;                // Initial brightness level of the OLED screen
unsigned long lastWiFiReconnect=0;  // Last WiFi reconnection time (millis)
bool firstLoad = true;              // Are we loading for the first time (no station config)?
int prevProgressBarPosition=0;      // Used for progress bar smooth animation
int startupProgressPercent;         // Initialisation progress
bool wifiConnected = false;         // Connected to WiFi?
unsigned long nextDataUpdate = 0;   // Next National Rail update time (millis)
int dataLoadSuccess = 0;            // Count of successful data downloads
int dataLoadFailure = 0;            // Count of failed data downloads
unsigned long lastLoadFailure = 0;  // When the last failure occurred
int dateWidth;                      // Width of the displayed date in pixels
int dateDay;                        // Day of the month of displayed date
bool altStationEnabled = false;     // Switch between stations based on time of day
bool altStationActive = false;      // Is the alternate station currently shown
byte altStarts = 12;                // Hour at which to switch to the alternate station
byte altEnds = 23;                  // Hour at which to switch back to the default station
bool noScrolling = false;           // Suppress all horizontal scrolling
bool flipScreen = false;            // Rotate screen 180deg
String timezone = "";               // custom (non UK) timezone for the clock
bool hidePlatform = false;          // Hide platform numbers on Rail board?
int nrTimeOffset = 0;               // Offset minutes for Rail departures display
int prevUpdateCheckDay;             // Day of the month the last daily firmware update check was made
unsigned long fwUpdateCheckTimer=0; // Next time to check if the day has rolled over for firmware update check
bool apiKeys = false;               // Does apikeys.json exist?

char hostname[33];                  // Network hostname (mDNS)
char myUrl[24];                     // Stores the board's own url

// WiFi Manager status
bool wifiConfigured = false;        // Is WiFi configured successfully?

// Station Board Data
char nrToken[37] = "";              // National Rail Darwin Lite Tokens are in the format nnnnnnnn-nnnn-nnnn-nnnn-nnnnnnnnnnnn, where each 'n' represents a hexadecimal character (0-9 or a-f).
char crsCode[4] = "";               // Station code (3 character)
float stationLat=0;                 // Selected station Latitude/Longitude (used to get weather for the location)
float stationLon=0;
char callingCrsCode[4] = "";        // Station code to filter routes on
char callingStation[45] = "";       // Calling filter station friendly name
char platformFilter[MAXPLATFORMFILTERSIZE]; // CSV list of platforms to filter on
char cleanPlatformFilter[MAXPLATFORMFILTERSIZE]; // Cleaned up platform filter (for performance)
char altCrsCode[4] = "";            // Station code of alternate station
float altLat=0;                     // Alternate station Latitude/Longitude (used to get weather for the location)
float altLon=0;
char altCallingCrsCode[4];          // Station code to filter routes on (when alternate station active)
char altCallingStation[45] = "";    // Calling filter station friendly name (when alternate station active)
char altPlatformFilter[MAXPLATFORMFILTERSIZE]; // CSV list of platforms to filter on
String tflAppkey = "";              // TfL API Key
char tubeId[13] = "";               // Underground station naptan id
String tubeName="";                 // Underground Station Name
char busAtco[13]="";                // Bus Stop ATCO location
String busName="";                  // Bus Stop long name
int busDestX;                       // Variable margin for bus destination
char busFilter[MAXBUSFILTERSIZE]=""; // CSV list of services to filter on
char cleanBusFilter[MAXBUSFILTERSIZE]; // Cleaned up bus filter (for performance)
float busLat=0;                     // Bus stop Latitude/Longitude (used to get weather for the location)
float busLon=0;

// board has three possible modes.
enum boardModes {
  MODE_RAIL = 0,
  MODE_TUBE = 1,
  MODE_BUS = 2
};
boardModes boardMode = MODE_RAIL;

// Coach class availability
static const char firstClassSeating[] PROGMEM = " First class seating only.";
static const char standardClassSeating[] PROGMEM = " Standard class seating only.";
static const char dualClassSeating[] PROGMEM = " First and Standard class seating available.";

// Animation vars
int numMessages=0;
int scrollStopsXpos = 0;
int scrollStopsYpos = 0;
int scrollStopsLength = 0;
bool isScrollingStops = false;
int currentMessage = 0;
int prevMessage = 0;
int prevScrollStopsLength = 0;
char line2[4+MAXBOARDMESSAGES][MAXCALLINGSIZE+12];

// Line 3 (additional services)
int line3Service = 0;
int scrollServiceYpos = 0;
bool isScrollingService = false;
int prevService = 0;
bool isShowingVia=false;
unsigned long serviceTimer=0;
unsigned long viaTimer=0;
bool showingMessage = false;
// TfL/bus specific animation
int scrollPrimaryYpos = 0;
bool isScrollingPrimary = false;
bool attributionScrolled = false;

char displayedTime[29] = "";        // The currently displayed time
unsigned long nextClockUpdate = 0;  // Next time we need to check/update the clock display
int fpsDelay=25;                    // Total ms between text movement (for smooth animation)
unsigned long refreshTimer = 0;

// Weather Stuff
char weatherMsg[46];                            // Current weather at station location
unsigned long nextWeatherUpdate = 0;            // When the next weather update is due
String openWeatherMapApiKey = "";               // The API key to use
weatherClient currentWeather;                   // Create a weather client

// RSS Client
rssClient rss;                                  // Create a RSS client
bool rssEnabled = false;                        // Add RSS feed to the messages
unsigned long nextRssUpdate = 0;                // When the next RSS update is due
bool rssAddedtoMsgs = false;
int lastRssUpdateResult = 0;
String rssURL;                                  // RSS URL to use
String rssName;                                 // Name of feed for atrribution

bool noDataLoaded = true;                       // True if no data received for the station
int lastUpdateResult = 0;                       // Result of last data refresh
unsigned long lastDataLoadTime = 0;             // Timestamp of last data load
long apiRefreshRate = DATAUPDATEINTERVAL;       // User selected refresh rate for National Rail API

#define MAXHOSTSIZE 48                          // Maximum size of the wsdl Host
#define MAXAPIURLSIZE 48                        // Maximum size of the wsdl url

char wsdlHost[MAXHOSTSIZE];                     // wsdl Host name
char wsdlAPI[MAXAPIURLSIZE];                    // wsdl API url

// RailData XML Client
raildataXmlClient* raildata = nullptr;
// TfL Client
TfLdataClient* tfldata = nullptr;
// Bus Client
busDataClient* busdata = nullptr;
// Station Data (shared)
rdStation station;
// Station Messages (shared)
stnMessages messages;

#include "gfx/DisplayEngine.hpp"

#include "webgui/WebHandlers.hpp"

//
void setup(void) {
  // These are the default wsdl XML SOAP entry points. They can be overridden in the config.json file if necessary
  strncpy(wsdlHost,"lite.realtime.nationalrail.co.uk",sizeof(wsdlHost));
  strncpy(wsdlAPI,"/OpenLDBWS/wsdl.aspx?ver=2021-11-01",sizeof(wsdlAPI));
  u8g2.begin();                       // Start the OLED panel
  u8g2.setContrast(brightness);       // Initial brightness
  u8g2.setDrawColor(1);               // Only a monochrome display, so set the colour to "on"
  u8g2.setFontMode(1);                // Transparent fonts
  u8g2.setFontRefHeightAll();         // Count entire font height
  u8g2.setFontPosTop();               // Reference from top
  u8g2.setFont(NatRailTall12);
  String buildDate = String(__DATE__);
  String notice = "\x80 " + buildDate.substring(buildDate.length()-4) + F(" Gadec Software (github.com/gadec-uk)");

  bool isFSMounted = LittleFS.begin(true);    // Start the File System, format if necessary
  strcpy(station.location,"");                // No default location
  strcpy(weatherMsg,"");                      // No weather message
  strcpy(nrToken,"");                         // No default National Rail token
  tflAppkey="";                               // No default TfL AppKey
  loadApiKeys();                              // Load the API keys from the apiKeys.json
  loadConfig();                               // Load the configuration settings from config.json
  u8g2.setContrast(brightness);               // Set the panel brightness to the user saved level
  if (flipScreen) u8g2.setFlipMode(1);
  u8g2.clearBuffer();
  u8g2.drawXBM(81,0,gadeclogo_width,gadeclogo_height,gadeclogo_bits);
  centreText(notice.c_str(),48);
  u8g2.sendBuffer();
  delay(5000);

  u8g2.clearBuffer();
  drawStartupHeading();
  u8g2.sendBuffer();
  progressBar(F("Connecting to Wi-Fi"),20);
  WiFi.mode(WIFI_MODE_NULL);        // Reset the WiFi
  WiFi.setSleep(WIFI_PS_NONE);      // Turn off WiFi Powersaving
  WiFi.hostname(hostname);          // Set the hostname ("Departures Board")
  WiFi.mode(WIFI_STA);              // Enter WiFi station mode
  WiFiManager wm;                   // Start WiFiManager
  wm.setAPCallback(wmConfigModeCallback);     // Set the callback for config mode notification
  wm.setWiFiAutoReconnect(true);              // Attempt to auto-reconnect WiFi
  wm.setConnectTimeout(8);
  wm.setConnectRetries(2);

  bool result = wm.autoConnect("Departures Board");    // Attempt to connect to WiFi (or enter interactive configuration mode)
  if (!result) {
      // Failed to connect/configure
      ESP.restart();
  }

  // Wait for WiFi connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
  }

  // Get our IP address and store
  updateMyUrl();
  if (MDNS.begin(hostname)) {
    MDNS.addService("http","tcp",80);
  }

  wifiConnected=true;
  WiFi.setAutoReconnect(true);
  u8g2.clearBuffer();                                             // Clear the display
  drawStartupHeading();                                           // Draw the startup heading
  char ipBuff[17];
  WiFi.localIP().toString().toCharArray(ipBuff,sizeof(ipBuff));   // Get the IP address of the ESP32
  centreText(ipBuff,53);                                          // Display the IP address
  progressBar(F("Wi-Fi Connected"),30);
  u8g2.sendBuffer();                                              // Send to OLED panel

  // Configure the local webserver paths
  server.on(F("/"),handleRoot);
  server.on(F("/erasewifi"),handleEraseWiFi);
  server.on(F("/factoryreset"),handleFactoryReset);
  server.on(F("/info"),handleInfo);
  server.on(F("/formatffs"),handleFormatFFS);
  server.on(F("/dir"),handleFileList);
  server.onNotFound(handleNotFound);
  server.on(F("/cat"),handleCat);
  server.on(F("/del"),handleDelete);
  server.on(F("/reboot"),handleReboot);
  server.on(F("/stationpicker"),handleStationPicker);           // Used by the Web GUI to lookup station codes interactively
  server.on(F("/firmware"),handleFirmwareInfo);                 // Used by the Web GUI to display the running firmware version
  server.on(F("/savesettings"),HTTP_POST,handleSaveSettings);   // Used by the Web GUI to save updated configuration settings
  server.on(F("/savekeys"),HTTP_POST,handleSaveKeys);           // Used by the Web GUI to verify/save API keys
  server.on(F("/brightness"),handleBrightness);                 // Used by the Web GUI to interactively set the panel brightness
  server.on(F("/ota"),handleOtaUpdate);                         // Used by the Web GUI to initiate a manual firmware/WebApp update
  server.on(F("/control"),handleControl);                       // Endpoint for automation

  server.on(F("/update"), HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, contentTypeHtml, updatePage);
  });
  /*handling uploading firmware file */
  server.on(F("/update"), HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    sendResponse(200,(Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        //Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        //Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
      } else {
        //Update.printError(Serial);
      }
    }
  });

  server.on(F("/upload"), HTTP_GET, []() {
      server.send(200, contentTypeHtml, uploadPage);
  });
  server.on(F("/upload"), HTTP_POST, []() {
  }, handleFileUpload);

  server.on(F("/success"), HTTP_GET, []() {
    server.send(200, contentTypeHtml, successPage);
  });

  server.begin();     // Start the local web server

  // Check for Firmware updates?
  if (firmwareUpdates) {
    progressBar(F("Checking for firmware updates"),40);
    if (ghUpdate.getLatestRelease()) {
      checkForFirmwareUpdate();
    } else {
      for (int i=15;i>=0;i--) {
        showUpdateCompleteScreen("Firmware Update Check Failed","Unable to retrieve latest release information.",ghUpdate.getLastError().c_str(),"",i,false);
        delay(1000);
      }
      u8g2.clearDisplay();
      drawStartupHeading();
      u8g2.sendBuffer();
    }
  }
  checkPostWebUpgrade();

  // First time configuration?
  if ((!crsCode[0] && !tubeId[0] && !busAtco[0]) || (!nrToken[0] && boardMode==MODE_RAIL)) {
    if (!apiKeys) showSetupKeysHelpScreen();
    else showSetupCrsHelpScreen();
    // First time setup mode will exit with a reboot, so just loop here forever servicing web requests
    while (true) {
      yield();
      server.handleClient();
    }
  }

  configTime(0,0, ntpServer);   // Configure NTP server for setting the clock
  setenv("TZ",ukTimezone,1);    // Configure UK TimeZone (default and fallback if custom is invalid)
  tzset();                      // Set the TimeZone
  if (timezone!="") {
    setenv("TZ",timezone.c_str(),1);
    tzset();
  }

  // Check the clock has been set successfully before continuing
  int p=50;
  int ntpAttempts=0;
  bool ntpResult=true;
  progressBar(F("Setting the system clock..."),50);
  if(!getLocalTime(&timeinfo)) {              // attempt to set the clock from NTP
    do {
      delay(500);                             // If no NTP response, wait 500ms and retry
      ntpResult = getLocalTime(&timeinfo);
      ntpAttempts++;
      p+=5;
      progressBar(F("Setting the system clock..."),p);
      if (p>80) p=45;
    } while ((!ntpResult) && (ntpAttempts<10));
  }
  if (!ntpResult) {
    // Sometimes NTP/UDP fails. A reboot usually fixes it.
    progressBar(F("Failed to set the clock. Rebooting in 5 sec."),0);
    delay(5000);
    ESP.restart();
  }
  prevUpdateCheckDay = timeinfo.tm_mday;

  station.numServices=0;
  if (rssEnabled && boardMode!=MODE_BUS) {
    progressBar(F("Loading RSS headlines feed"),60);
    updateRssFeed();
  }

  if (weatherEnabled && boardMode!=MODE_TUBE) {
    progressBar(F("Getting weather conditions"),64);
    updateCurrentWeather(stationLat,stationLon);
  }

  if (boardMode == MODE_RAIL) {
      progressBar(F("Initialising National Rail interface"),67);
      altStationActive = setAlternateStation();  // Check & set the alternate station if appropriate
      raildata = new raildataXmlClient();
      int res = raildata->init(wsdlHost, wsdlAPI, &raildataCallback);
      if (res != UPD_SUCCESS) {
        showWsdlFailureScreen();
        while (true) { server.handleClient(); yield();}
      }
      progressBar(F("Initialising National Rail interface"),70);
      raildata->cleanFilter(platformFilter,cleanPlatformFilter,sizeof(platformFilter));
  } else if (boardMode == MODE_TUBE) {
      progressBar(F("Initialising TfL interface"),70);
      tfldata = new TfLdataClient();
      startupProgressPercent=70;
  } else if (boardMode == MODE_BUS) {
      progressBar(F("Initialising BusTimes interface"),70);
      busdata = new busDataClient();
      // Create a cleaned filter
      busdata->cleanFilter(busFilter,cleanBusFilter,sizeof(busFilter));
      startupProgressPercent=70;
  }
}


/**
 * @brief Loop
 */
void loop(void) {

  // Check for firmware updates daily if enabled
  if (dailyUpdateCheck && millis()>fwUpdateCheckTimer) {
    fwUpdateCheckTimer = millis() + 3300000 + random(600000); // check again in 55 to 65 mins
    if (getLocalTime(&timeinfo)) {
      if (timeinfo.tm_mday != prevUpdateCheckDay) {
        if (ghUpdate.getLatestRelease()) {
          checkForFirmwareUpdate();
          prevUpdateCheckDay = timeinfo.tm_mday;
        }
      }
    }
  }

  bool wasSleeping = isSleeping;
  isSleeping = isSnoozing();

  if (isSleeping && millis()>timer) {       // If the "screensaver" is active, change the screen every 8 seconds
    drawSleepingScreen();
    timer=millis()+8000;
  } else if (wasSleeping && !isSleeping) {
    // Exit sleep mode cleanly
    firstLoad=true;
    nextDataUpdate=0;
    isScrollingStops=false;
    isScrollingService=false;
    isScrollingPrimary=false;
    prevProgressBarPosition=70;
    u8g2.clearDisplay();
  }

  // WiFi Status icon
  if (WiFi.status() != WL_CONNECTED && wifiConnected) {
    wifiConnected=false;
    u8g2.setFont(NatRailSmall9);
    u8g2.drawStr(0,56,"\x7F");  // No Wifi Icon
    u8g2.updateDisplayArea(0,7,1,1);
  } else if (WiFi.status() == WL_CONNECTED && !wifiConnected) {
    wifiConnected=true;
    blankArea(0,57,5,7);
    u8g2.updateDisplayArea(0,7,1,1);
    updateMyUrl();  // in case our IP changed
  }

  // Force a manual reset if we've been disconnected for more than 10 secs
  if (WiFi.status() != WL_CONNECTED && millis() > lastWiFiReconnect+10000) {
    WiFi.disconnect();
    delay(100);
    WiFi.reconnect();
    lastWiFiReconnect=millis();
  }

  switch (boardMode) {
    case MODE_RAIL:
      departureBoardLoop();
      break;

    case MODE_TUBE:
      undergroundArrivalsLoop();
      break;

    case MODE_BUS:
      busDeparturesLoop();
      break;
  }

  server.handleClient();
}
