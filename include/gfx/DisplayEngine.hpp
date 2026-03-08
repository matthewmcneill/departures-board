/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: gfx/DisplayEngine.hpp
 * Description: Contains all the Core UI rendering functions for the OLED screen,
 *              as well as logic for screen states, updates, and animations.
 *
 * Exported Functions/Classes:
 * - blankArea: Clear a region
 * - getStringWidth: Get pixel width of text
 * - drawTruncatedText/centreText: Text rendering utilities
 * - progressBar/drawProgressBar: Loading indicators
 * - drawBuildTime/drawStartupHeading: Startup screens
 * - drawStationHeader: Draw top header
 * - drawCurrentTime/drawCurrentTimeUG: Clock logic
 * - showSetupScreen/showNoDataScreen/showWsdlFailureScreen/etc: Error screens
 * - saveFile/loadFile: Config helpers
 * - loadConfig/loadApiKeys/writeDefaultConfig: Configuration parsers
 * - isSnoozing/isAltActive/doClockCheck: Core logic states
 * - getStationBoard/drawStationBoard/drawPrimaryService/drawServiceLine: National Rail
 * - getUndergroundBoard/drawUndergroundBoard/drawUndergroundService: TfL Tube
 * - getBusDeparturesBoard/drawBusDeparturesBoard/drawBusService: Bus Arrivals
 */

/*
 * Graphics helper functions for OLED panel
*/
/**
 * @brief Clear a rectangular area on the OLED display by drawing a black box
 * @param x Top left X coordinate
 * @param y Top left Y coordinate
 * @param w Width of the area
 * @param h Height of the area
 */
void blankArea(int x, int y, int w, int h) {
  u8g2.setDrawColor(0);
  u8g2.drawBox(x,y,w,h);
  u8g2.setDrawColor(1);
}

/**
 * @brief Calculate the width in pixels of a string using the current font
 * @param message C-string to measure
 * @return Width in pixels
 */
int getStringWidth(const char *message) {
  return u8g2.getStrWidth(message);
}

/**
 * @brief Calculate the width in pixels of a PROGMEM string using the current font
 * @param message Flash string to measure
 * @return Width in pixels
 */
int getStringWidth(const __FlashStringHelper *message) {
  String temp = String(message);
  char buff[temp.length()+1];
  temp.toCharArray(buff,sizeof(buff));
  return u8g2.getStrWidth(buff);
}

/**
 * @brief Draw text left-aligned, truncating with '...' if it exceeds screen width
 * @param message The text to display
 * @param line The Y coordinate for the baseline
 */
void drawTruncatedText(const char *message, int line) {
  char buff[strlen(message)+4];
  int maxWidth = SCREEN_WIDTH - 6;
  strcpy(buff,message);
  int i = strlen(buff);
  while (u8g2.getStrWidth(buff)>maxWidth && i) buff[i--] = '\0';
  strcat(buff,"...");
  u8g2.drawStr(0,line,buff);
}

/**
 * @brief Draw text horizontally centered on the screen
 * @param message The text to display
 * @param line The Y coordinate for the baseline
 */
void centreText(const char *message, int line) {
  int width = u8g2.getStrWidth(message);
  if (width<=SCREEN_WIDTH) u8g2.drawStr((SCREEN_WIDTH-width)/2,line,message);
  else drawTruncatedText(message,line);
}

/**
 * @brief Draw PROGMEM text horizontally centered on the screen
 * @param message The PROGMEM text to display
 * @param line The Y coordinate for the baseline
 */
void centreText(const __FlashStringHelper *message, int line) {
  String temp = String(message);
  char buff[temp.length()+1];
  temp.toCharArray(buff,sizeof(buff));
  int width = u8g2.getStrWidth(buff);
  if (width<=SCREEN_WIDTH) u8g2.drawStr((SCREEN_WIDTH-width)/2,line,buff);
  else drawTruncatedText(buff,line);
}

/**
 * @brief Draw a progress bar at the bottom of the screen
 * @param percent Progress amount (0-100)
 */
void drawProgressBar(int percent) {
  int newPosition = (percent*190)/100;
  u8g2.drawFrame(32,36,192,12);
  if (prevProgressBarPosition>newPosition) {
    for (int i=prevProgressBarPosition;i>=newPosition;i--) {
      u8g2.setDrawColor(0);
      u8g2.drawBox(33,37,190,10);
      u8g2.setDrawColor(1);
      u8g2.drawBox(33,37,i,10);
      u8g2.updateDisplayArea(0,3,32,3);
      delay(5);
    }
  } else {
    for (int i=prevProgressBarPosition;i<=newPosition;i++) {
      u8g2.setDrawColor(0);
      u8g2.drawBox(33,37,190,10);
      u8g2.setDrawColor(1);
      u8g2.drawBox(33,37,i,10);
      u8g2.updateDisplayArea(0,3,32,3);
      delay(5);
    }
  }
  prevProgressBarPosition=newPosition;
}

/**
 * @brief Draw centered status text and update the progress bar
 * @param text Status description
 * @param percent Progress amount (0-100)
 */
void progressBar(const char *text, int percent) {
  u8g2.setFont(NatRailSmall9);
  blankArea(0,24,256,25);
  centreText(text,24);
  drawProgressBar(percent);
}

/**
 * @brief Draw centered PROGMEM status text and update the progress bar
 * @param text PROGMEM Status description
 * @param percent Progress amount (0-100)
 */
void progressBar(const __FlashStringHelper *text, int percent) {
  u8g2.setFont(NatRailSmall9);
  blankArea(0,24,256,25);
  centreText(text,24);
  drawProgressBar(percent);
}

/**
 * @brief Draw the firmware compile build date and time at the bottom right
 */
void drawBuildTime() {
  char timestamp[22];
  char buildtime[20];
  struct tm tm = {};

  sprintf(timestamp,"%s %s",__DATE__,__TIME__);
  strptime(timestamp,"%b %d %Y %H:%M:%S",&tm);
  sprintf(buildtime,"v%d.%d-%02d%02d%02d%02d%02d",VERSION_MAJOR,VERSION_MINOR,tm.tm_year-100,tm.tm_mon+1,tm.tm_mday,tm.tm_hour,tm.tm_min);
  u8g2.drawStr(0,53,buildtime);
}

/**
 * @brief Draw the main Departures Board intro logo and heading
 */
void drawStartupHeading() {
  u8g2.setFont(NatRailTall12);
  centreText(F("Departures Board"),0);
  u8g2.setFont(NatRailSmall9);
  drawBuildTime();
}

/**
 * @brief Draw the National Rail header row including station name and clock
 * @param stopName Primary station name
 * @param callingStopName Filter/via station name
 * @param platFilter Active platform filter
 * @param timeOffset Time offset applied to board
 */
void drawStationHeader(const char *stopName, const char *callingStopName, const char *platFilter, const int timeOffset) {

  // Clear the top line
  if (boardMode == MODE_TUBE || boardMode == MODE_BUS) {
    blankArea(0,ULINE0,256,ULINE1-1);
  } else {
    blankArea(0,LINE0,256,LINE1-1);
  }

  u8g2.setFont(NatRailSmall9);
  char boardTitle[95];
  String title = String(stopName) + " ";
  if (timeOffset) {
    title+="\x8F";
    if (timeOffset>0) title+="+";
    title+=String(timeOffset) + "m ";
  }
  if (platFilter[0]) title+="\x8D" + String(platFilter) + " ";
  if (callingStopName[0]) title+="(\x81" + String(callingStopName) + ")";
  title.trim();
  strncpy(boardTitle,title.c_str(),sizeof(boardTitle));

  int boardTitleWidth = getStringWidth(boardTitle);

  if (dateEnabled) {
    int const dateY=55;
    // Get the date
    char sysTime[29];
    getLocalTime(&timeinfo);
    strftime(sysTime,29,"%a %d %b",&timeinfo);
    dateWidth = getStringWidth(sysTime);
    dateDay = timeinfo.tm_mday;
    if (callingStopName[0] || boardTitleWidth+dateWidth+10>=SCREEN_WIDTH) {
      blankArea(SCREEN_WIDTH-70,dateY,70,SCREEN_HEIGHT-dateY);
      u8g2.drawStr(SCREEN_WIDTH-dateWidth,dateY-1,sysTime); // Date bottom right
      centreText(boardTitle,LINE0-1);
    } else {
      u8g2.drawStr(SCREEN_WIDTH-dateWidth,LINE0-1,sysTime); // right-aligned date top
      if ((SCREEN_WIDTH-boardTitleWidth)/2 < dateWidth+8) {
        // station name left aligned
        u8g2.drawStr(0,LINE0-1,boardTitle);
      } else {
        centreText(boardTitle,LINE0-1);
      }
    }
  } else {
    centreText(boardTitle,LINE0-1);
  }
}

/**
 * @brief Render the current time in the top right corner for National Rail boards
 * @param update If true, visually refresh the display area immediately
 */
void drawCurrentTime(bool update) {
  char sysTime[29];
  getLocalTime(&timeinfo);

  sprintf(sysTime,"%02d:%02d:",timeinfo.tm_hour,timeinfo.tm_min);
  if (strcmp(displayedTime,sysTime)) {
    u8g2.setFont(NatRailClockLarge9);
    blankArea(96,LINE4,64,SCREEN_HEIGHT-LINE4);
    u8g2.drawStr(96,LINE4-1,sysTime);
    u8g2.setFont(NatRailClockSmall7);
    sprintf(sysTime,"%02d",timeinfo.tm_sec);
    u8g2.drawStr(144,LINE4+1,sysTime);
    u8g2.setFont(NatRailSmall9);
    if (update) u8g2.updateDisplayArea(12,6,8,2);
    strcpy(displayedTime,sysTime);
    if (dateEnabled && timeinfo.tm_mday!=dateDay) {
      // Need to update the date on screen
      drawStationHeader(station.location,callingStation,platformFilter,nrTimeOffset);
      if (update) u8g2.sendBuffer();  // Just refresh on new date
    }
  }
}

/**
 * @brief Render the screensaver / sleep mode UI (moving clock or logo)
 */
void drawSleepingScreen() {
  char sysTime[8];
  char sysDate[29];

  u8g2.setContrast(DIMMED_BRIGHTNESS);
  u8g2.clearBuffer();
  if (sleepClock) {
    getLocalTime(&timeinfo);
    sprintf(sysTime,"%02d:%02d",timeinfo.tm_hour,timeinfo.tm_min);
    strftime(sysDate,29,"%d %B %Y",&timeinfo);

    int offset = (getStringWidth(sysDate)-getStringWidth(sysTime))/2;
    u8g2.setFont(NatRailTall12);
    int y = random(39);
    int x = random(SCREEN_WIDTH-getStringWidth(sysDate));
    u8g2.drawStr(x+offset,y,sysTime);
    u8g2.setFont(NatRailSmall9);
    u8g2.drawStr(x,y+13,sysDate);
  }
  u8g2.sendBuffer();
}

/**
 * @brief Show or hide the OTA update available icon
 * @param show true to show, false to clear
 */
void showUpdateIcon(bool show) {
  if (show) {
    u8g2.setFont(NatRailTall12);
    u8g2.drawStr(0,50,"}");
    u8g2.setFont(NatRailSmall9);
  } else {
    blankArea(0,50,6,13);
  }
  u8g2.updateDisplayArea(0,6,1,2);
}

/*
 * Setup / Notification Screen Layouts
*/

/**
 * @brief Render the initial configuration wizard instructions via WiFi
 */
void showSetupScreen() {
  u8g2.clearBuffer();
  u8g2.setFont(NatRailTall12);
  centreText(F("Departures Board first-time setup"),0);
  u8g2.setFont(NatRailSmall9);
  centreText(F("To configure Wi-Fi, please connect to the"),18);
  centreText(F("the \"Departures Board\" network and go to"),32);
  centreText(F("http://192.168.4.1 in a web browser."),46);
  u8g2.sendBuffer();
}

/**
 * @brief Render an error screen when no data could be fetched from the API
 */
void showNoDataScreen() {
  u8g2.clearBuffer();
  char msg[60];
  u8g2.setFont(NatRailTall12);
  switch (boardMode) {
    case MODE_RAIL:
      sprintf(msg,"No data available for station code \"%s\".",crsCode);
      break;
    case MODE_TUBE:
      strcpy(msg,"No data available for the selected station.");
      break;
    case MODE_BUS:
      strcpy(msg,"No data available for the selected bus stop.");
      break;
  }
  centreText(msg,-1);
  u8g2.setFont(NatRailSmall9);
  centreText(F("Please check you have selected a valid location"),14);
  centreText(F("Go to the URL below to choose a location..."),26);
  centreText(myUrl,40);
  u8g2.sendBuffer();
}

/**
 * @brief Show setup keys help screen
 */
void showSetupKeysHelpScreen() {
  u8g2.clearBuffer();
  char msg[60];
  u8g2.setFont(NatRailTall12);
  centreText(F("Departures Board Setup"),-1);
  u8g2.setFont(NatRailSmall9);
  centreText(F("Next, you need to enter your API keys."),16);
  centreText(F("Please go to the URL below to start..."),28);
  u8g2.setFont(NatRailTall12);
  centreText(myUrl,50);
  u8g2.sendBuffer();
}

/**
 * @brief Show setup crs help screen
 */
void showSetupCrsHelpScreen() {
  u8g2.clearBuffer();
  char msg[60];
  u8g2.setFont(NatRailTall12);
  centreText(F("Departures Board Setup"),-1);
  u8g2.setFont(NatRailSmall9);
  centreText(F("Next, you need to choose a location. Please"),16);
  centreText(F("go to the URL below to choose a station..."),28);
  u8g2.setFont(NatRailTall12);
  centreText(myUrl,50);
  u8g2.sendBuffer();
}

/**
 * @brief Render a critical error if National Rail WSDL fails to init
 */
void showWsdlFailureScreen() {
  u8g2.clearBuffer();
  u8g2.setFont(NatRailTall12);
  centreText(F("The National Rail data feed is unavailable."),-1);
  u8g2.setFont(NatRailSmall9);
  centreText(F("WDSL entry point could not be accessed, so the"),14);
  centreText(F("Departures Board cannot be loaded."),26);
  centreText(F("Please try again later. :("),40);
  u8g2.sendBuffer();
}

/**
 * @brief Render an error meaning the National Rail or TfL token is invalid
 */
void showTokenErrorScreen() {
  char msg[60];
  u8g2.clearBuffer();
  u8g2.setFont(NatRailTall12);
  switch (boardMode) {
    case MODE_RAIL:
      centreText(F("Access to the National Rail database denied."),-1);
      strcpy(nrToken,"");
      break;
    case MODE_TUBE:
      centreText(F("Access to the TfL database denied."),-1);
      break;
    case MODE_BUS:
      centreText(F("Access to the bustimes database denied."),-1);
      break;
  }
  u8g2.setFont(NatRailSmall9);
  centreText(F("You must enter a valid auth token, please"),14);
  centreText(F("check you have entered it correctly below:"),26);
  sprintf(msg,"%s/keys.htm",myUrl);
  centreText(msg,40);
  u8g2.sendBuffer();
}

/**
 * @brief Show c r s error screen
 */
void showCRSErrorScreen() {
  u8g2.clearBuffer();
  char msg[60];
  u8g2.setFont(NatRailTall12);
  switch (boardMode) {
    case MODE_RAIL:
      sprintf(msg,"The station code \"%s\" is not valid.",crsCode);
      break;
    case MODE_TUBE:
      strcpy(msg,"The Underground station is not valid");
      break;
    case MODE_BUS:
      sprintf(msg,"The atco code \"%s\" is not valid.",busAtco);
      break;
  }
  centreText(msg,-1);
  u8g2.setFont(NatRailSmall9);
  centreText(F("Please ensure you have selected a valid station."),14);
  centreText(F("Go to the URL below to choose a station..."),26);
  centreText(myUrl,40);
  u8g2.sendBuffer();
}

/**
 * @brief Show firmware update warning screen
 * @param msg
 * @param secs
 */
void showFirmwareUpdateWarningScreen(const char *msg, int secs) {
  char countdown[60];
  u8g2.clearBuffer();
  u8g2.setFont(NatRailTall12);
  centreText(F("Firmware Update Available"),-1);
  u8g2.setFont(NatRailSmall9);
  centreText(F("A new version of the Departures Board firmware"),14);
  sprintf(countdown,"will be installed in %d seconds. This provides:",secs);
  centreText(countdown,26);
  sprintf(countdown,"\"%s\"",msg);
  centreText(countdown,40);
  centreText(F("* DO NOT REMOVE THE POWER DURING THE UPDATE *"),54);
  u8g2.sendBuffer();
}

/**
 * @brief Show firmware update progress
 * @param percent
 */
void showFirmwareUpdateProgress(int percent) {
  u8g2.clearBuffer();
  u8g2.setFont(NatRailTall12);
  centreText(F("Firmware Update in Progress"),-1);
  u8g2.setFont(NatRailSmall9);
  progressBar(F("Updating Firmware"),percent);
  centreText(F("* DO NOT REMOVE THE POWER DURING THE UPDATE *"),54);
  u8g2.sendBuffer();
}

/**
 * @brief Show update complete screen
 * @param title
 * @param msg1
 * @param msg2
 * @param msg3
 * @param secs
 * @param showReboot
 */
void showUpdateCompleteScreen(const char *title, const char *msg1, const char *msg2, const char *msg3, int secs, bool showReboot) {
  char countdown[60];
  u8g2.clearBuffer();
  u8g2.setFont(NatRailTall12);
  centreText(title,-1);
  u8g2.setFont(NatRailSmall9);
  centreText(msg1,14);
  centreText(msg2,26);
  centreText(msg3,40);
  if (showReboot) sprintf(countdown,"The system will restart in %d seconds.",secs);
  else sprintf(countdown,"The system will continue in %d seconds.",secs);
  centreText(countdown,54);
  u8g2.sendBuffer();
}

/*
 * Utility functions
*/

/**
 * @brief Utility to save string data to a file in LittleFS
 * @param fName Path to the file
 * @param fData Content to save
 * @return true if successful
 */
bool saveFile(String fName, String fData) {
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
 * @brief Utility to load text from a file into a String object
 * @param fName Path to the file
 * @return File contents or empty string if failed
 */
String loadFile(String fName) {
  File f = LittleFS.open(fName,"r");
  if (f) {
    String result = f.readString();
    f.close();
    return result;
  } else return "";
}

// Get the Build Timestamp of the running firmware
String getBuildTime() {
  char timestamp[22];
  char buildtime[11];
  struct tm tm = {};

  sprintf(timestamp,"%s %s",__DATE__,__TIME__);
  strptime(timestamp,"%b %d %Y %H:%M:%S",&tm);
  sprintf(buildtime,"%02d%02d%02d%02d%02d",tm.tm_year-100,tm.tm_mon+1,tm.tm_mday,tm.tm_hour,tm.tm_min);
  return String(buildtime);
}

/**
 * @brief Check post web upgrade
 */
void checkPostWebUpgrade() {
  String prevGUI = loadFile(F("/webver"));
  prevGUI.trim();
  String currentGUI = String(WEBAPPVER_MAJOR) + F(".") + String(WEBAPPVER_MINOR);
  if (prevGUI != currentGUI) {
    // clean up old/dev files
    progressBar(F("Cleaning up following upgrade"),45);
    LittleFS.remove(F("/index_d.htm"));
    LittleFS.remove(F("/index.htm"));
    LittleFS.remove(F("/keys.htm"));
    LittleFS.remove(F("/nrelogo.webp"));
    LittleFS.remove(F("/tfllogo.webp"));
    LittleFS.remove(F("/btlogo.webp"));
    LittleFS.remove(F("/tube.webp"));
    LittleFS.remove(F("/nr.webp"));
    LittleFS.remove(F("/favicon.svg"));
    LittleFS.remove(F("/favicon.png"));
    saveFile(F("/webver"),currentGUI);
  }
}

// Check if the NR clock needs to be updated
void doClockCheck() {
  if (!firstLoad) {
    if (millis()>nextClockUpdate) {
      drawCurrentTime(true);
      nextClockUpdate=millis()+250;
    }
  }
}

// Returns true if sleep mode is enabled and we're within the sleep period
bool isSnoozing() {
  if (forcedSleep) return true;
  if (!sleepEnabled) return false;
  getLocalTime(&timeinfo);
  byte myHour = timeinfo.tm_hour;
  if (sleepStarts > sleepEnds) {
    if ((myHour >= sleepStarts) || (myHour < sleepEnds)) return true; else return false;
  } else {
    if ((myHour >= sleepStarts) && (myHour < sleepEnds)) return true; else return false;
  }
}

// Returns true if an alternate station is enabled and we're within the activation period
bool isAltActive() {
  if (!altStationEnabled) return false;
  getLocalTime(&timeinfo);
  byte myHour = timeinfo.tm_hour;
  if (altStarts > altEnds) {
    if ((myHour >= altStarts) || (myHour < altEnds)) return true; else return false;
  } else {
    if ((myHour >= altStarts) && (myHour < altEnds)) return true; else return false;
  }
}

// Callback from the raildataXMLclient library when processing data. As this can take some time, this callback is used to keep the clock working
// and to provide progress on the initial load at boot
void raildataCallback(int stage, int nServices) {
  if (firstLoad) {
    int percent = ((nServices*20)/MAXBOARDSERVICES)+80;
    progressBar(F("Initialising National Rail interface"),percent);
  } else doClockCheck();
}

// Stores/updates the url of our Web GUI
void updateMyUrl() {
  IPAddress ip = WiFi.localIP();
  snprintf(myUrl,sizeof(myUrl),"http://%u.%u.%u.%u",ip[0],ip[1],ip[2],ip[3]);
}

/*
 * Start-up configuration functions
 */

/**
 * @brief Load keys from apikeys.json
 */
void loadApiKeys() {
  LOG_INFO("Loading API keys from /apikeys.json...");
  JsonDocument doc;

  if (LittleFS.exists(F("/apikeys.json"))) {
    File file = LittleFS.open(F("/apikeys.json"), "r");
    if (file) {
      DeserializationError error = deserializeJson(doc, file);
      if (!error) {
        JsonObject settings = doc.as<JsonObject>();

        if (settings[F("nrToken")].is<const char*>()) {
          strlcpy(nrToken, settings[F("nrToken")], sizeof(nrToken));
        }

        if (settings[F("owmToken")].is<const char*>()) {
          openWeatherMapApiKey = settings[F("owmToken")].as<String>();
        }

        if (settings[F("appKey")].is<const char*>()) {
          tflAppkey = settings[F("appKey")].as<String>();
        }
        apiKeys = true;

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

// Write a default config file so that the Web GUI works initially (force Tube mode if no NR token)
void writeDefaultConfig() {
    String defaultConfig = "{\"crs\":\"\",\"station\":\"\",\"lat\":0,\"lon\":0,\"weather\":" + String((openWeatherMapApiKey.length())?"true":"false") + F(",\"sleep\":false,\"showDate\":false,\"showBus\":false,\"update\":false,\"sleepStarts\":23,\"sleepEnds\":8,\"brightness\":20,\"tubeId\":\"\",\"tubeName\":\"\",\"mode\":") + String((!nrToken[0])?"1":"0") + "}";
    saveFile(F("/config.json"),defaultConfig);
    strcpy(crsCode,"");
    strcpy(tubeId,"");
}

/**
 * @brief Load settings from config.json and populate global variables
 */
void loadConfig() {
  LOG_INFO("Loading configuration from /config.json...");
  JsonDocument doc;

  // Set defaults
  strcpy(hostname,defaultHostname);
  timezone = String(ukTimezone);

  if (LittleFS.exists(F("/config.json"))) {
    File file = LittleFS.open(F("/config.json"), "r");
    if (file) {
      DeserializationError error = deserializeJson(doc, file);
      if (!error) {
        JsonObject settings = doc.as<JsonObject>();

        if (settings[F("crs")].is<const char*>())        strlcpy(crsCode, settings[F("crs")], sizeof(crsCode));
        if (settings[F("callingCrs")].is<const char*>()) strlcpy(callingCrsCode, settings[F("callingCrs")], sizeof(callingCrsCode));
        if (settings[F("callingStation")].is<const char*>()) strlcpy(callingStation, settings[F("callingStation")], sizeof(callingStation));
        if (settings[F("platformFilter")].is<const char*>())  strlcpy(platformFilter, settings[F("platformFilter")], sizeof(platformFilter));
        if (settings[F("hostname")].is<const char*>())   strlcpy(hostname, settings[F("hostname")], sizeof(hostname));
        if (settings[F("wsdlHost")].is<const char*>())   strlcpy(wsdlHost, settings[F("wsdlHost")], sizeof(wsdlHost));
        if (settings[F("wsdlAPI")].is<const char*>())    strlcpy(wsdlAPI, settings[F("wsdlAPI")], sizeof(wsdlAPI));
        if (settings[F("showDate")].is<bool>())          dateEnabled = settings[F("showDate")];
        if (settings[F("showBus")].is<bool>())           enableBus = settings[F("showBus")];
        if (settings[F("sleep")].is<bool>())             sleepEnabled = settings[F("sleep")];
        if (settings[F("fastRefresh")].is<bool>())       apiRefreshRate = settings[F("fastRefresh")] ? FASTDATAUPDATEINTERVAL : DATAUPDATEINTERVAL;
        if (settings[F("weather")].is<bool>() && openWeatherMapApiKey.length())
                                                    weatherEnabled = settings[F("weather")];
        if (settings[F("update")].is<bool>())            firmwareUpdates = settings[F("update")];
        if (settings[F("updateDaily")].is<bool>())       dailyUpdateCheck = settings[F("updateDaily")];
        if (settings[F("sleepStarts")].is<int>())        sleepStarts = settings[F("sleepStarts")];
        if (settings[F("sleepEnds")].is<int>())          sleepEnds = settings[F("sleepEnds")];
        if (settings[F("brightness")].is<int>())         brightness = settings[F("brightness")];
        if (settings[F("lat")].is<float>())              stationLat = settings[F("lat")];
        if (settings[F("lon")].is<float>())              stationLon = settings[F("lon")];

        if (settings[F("mode")].is<int>())               boardMode = settings[F("mode")];
        else if (settings[F("tube")].is<bool>())         boardMode = settings[F("tube")] ? MODE_TUBE : MODE_RAIL; // handle legacy v1.x config
        if (settings[F("tubeId")].is<const char*>())     strlcpy(tubeId, settings[F("tubeId")], sizeof(tubeId));
        if (settings[F("tubeName")].is<const char*>())   tubeName = settings[F("tubeName")].as<String>();

        // Clean up the underground station name
        if (tubeName.endsWith(F(" Underground Station"))) tubeName.remove(tubeName.length()-20);
        else if (tubeName.endsWith(F(" DLR Station"))) tubeName.remove(tubeName.length()-12);
        else if (tubeName.endsWith(F(" (H&C Line)"))) tubeName.remove(tubeName.length()-11);

        if (settings[F("altCrs")].is<const char*>())     strlcpy(altCrsCode, settings[F("altCrs")], sizeof(altCrsCode));
        if (altCrsCode[0]) altStationEnabled = true; else altStationEnabled = false;
        if (settings[F("altStarts")].is<int>())          altStarts = settings[F("altStarts")];
        if (settings[F("altEnds")].is<int>())            altEnds = settings[F("altEnds")];
        if (settings[F("altLat")].is<float>())           altLat = settings[F("altLat")];
        if (settings[F("altLon")].is<float>())           altLon = settings[F("altLon")];
        if (settings[F("altCallingCrs")].is<const char*>()) strlcpy(altCallingCrsCode, settings[F("altCallingCrs")], sizeof(altCallingCrsCode));
        if (settings[F("altCallingStation")].is<const char*>()) strlcpy(altCallingStation, settings[F("altCallingStation")], sizeof(altCallingStation));
        if (settings[F("altPlatformFilter")].is<const char*>())  strlcpy(altPlatformFilter, settings[F("altPlatformFilter")], sizeof(altPlatformFilter));

        if (settings[F("busId")].is<const char*>())      strlcpy(busAtco, settings[F("busId")], sizeof(busAtco));
        if (settings[F("busName")].is<const char*>())    busName = String(settings[F("busName")]);
        if (settings[F("busLat")].is<float>())           busLat = settings[F("busLat")];
        if (settings[F("busLon")].is<float>())           busLon = settings[F("busLon")];
        if (settings[F("busFilter")].is<const char*>())  strlcpy(busFilter, settings[F("busFilter")], sizeof(busFilter));

        if (settings[F("noScroll")].is<bool>())          noScrolling = settings[F("noScroll")];
        if (settings[F("flip")].is<bool>())              flipScreen = settings[F("flip")];
        if (settings[F("TZ")].is<const char*>())         timezone = settings[F("TZ")].as<String>();
        if (settings[F("nrTimeOffset")].is<int>())       nrTimeOffset = settings[F("nrTimeOffset")];
        if (settings[F("hidePlatform")].is<bool>())      hidePlatform = settings[F("hidePlatform")];

        if (settings[F("rssUrl")].is<const char*>())     rssURL = String(settings[F("rssUrl")]);
        if (settings[F("rssName")].is<const char*>())    rssName = String(settings[F("rssName")]);
        if (rssURL != "") rssEnabled = true; else rssEnabled = false;

      } else {
        LOG_ERROR(String("Failed to parse /config.json: ") + error.c_str());
      }
      file.close();
    } else {
      LOG_ERROR("Failed to open /config.json for reading.");
    }
  } else {
    LOG_INFO("/config.json not found. Creating default config.");
    if (nrToken[0] || tflAppkey.length()) writeDefaultConfig();
  }
}

/**
 * @brief Check schedule and set alternate station variables if in time range
 * @return true if alternate station is currently active
 */
bool setAlternateStation() {
  if (boardMode==MODE_RAIL && altStationEnabled && isAltActive()) {
    // Switch to alternate station
    strcpy(crsCode,altCrsCode);
    stationLat = altLat;
    stationLon = altLon;
    strcpy(callingCrsCode, altCallingCrsCode);
    strcpy(callingStation, altCallingStation);
    strcpy(platformFilter, altPlatformFilter);
    return true;
  } else {
    return false;
  }
}

/**
 * @brief Update rss feed
 */
void updateRssFeed() {
  if (lastRssUpdateResult=rss.loadFeed(rssURL); lastRssUpdateResult == UPD_SUCCESS) nextRssUpdate = millis() + 600000; // update every ten minutes
  else nextRssUpdate = millis() + 300000; // Failed so try again in 5 minutes
}

/**
 * @brief Perform a gentle software reset to switch stations without full reboot
 */
void softResetBoard() {
  int previousMode = boardMode;
  String prevRssUrl = rssURL;

  // Reload the settings
  loadConfig();
  if (flipScreen) u8g2.setFlipMode(1); else u8g2.setFlipMode(0);
  if (timezone!="") {
    setenv("TZ",timezone.c_str(),1);
  } else {
    setenv("TZ",ukTimezone,1);
  }
  tzset();
  u8g2.clearBuffer();
  drawStartupHeading();
  u8g2.updateDisplay();

  // Force an update asap
  nextDataUpdate = 0;
  nextWeatherUpdate = 0;
  isScrollingService = false;
  isScrollingStops = false;
  isScrollingPrimary = false;
  isSleeping=false;
  firstLoad=true;
  noDataLoaded=true;
  viaTimer=0;
  timer=0;
  prevProgressBarPosition=70;
  startupProgressPercent=70;
  currentMessage=0;
  prevMessage=0;
  prevScrollStopsLength=0;
  isShowingVia=false;
  line3Service=0;
  prevService=0;
  if (!weatherEnabled) strcpy(weatherMsg,"");
  if (previousMode!=boardMode) {
    // Board mode has changed!
    switch (previousMode) {
      case MODE_RAIL:
        // Delete the NR client from memory
        delete raildata;
        raildata = nullptr;
        break;

      case MODE_TUBE:
        // Delete the tfl client from memory
        delete tfldata;
        tfldata = nullptr;
        break;

      case MODE_BUS:
        // Delete the Bus client from memory
        delete busdata;
        busdata = nullptr;
        break;
    }

    switch (boardMode) {
      case MODE_RAIL:
        // Create the NR client
        raildata = new raildataXmlClient();
        if (boardMode == MODE_RAIL) {
          int res = raildata->init(wsdlHost, wsdlAPI, &raildataCallback);
          if (res != UPD_SUCCESS) {
            showWsdlFailureScreen();
             while (true) { server.handleClient(); yield();}
          }
        }
        break;

      case MODE_TUBE:
        // Create the TfL client
        tfldata = new TfLdataClient();
        break;

      case MODE_BUS:
        // Create the Bus client
        busdata = new busDataClient();
        break;
    }
  }

  rssAddedtoMsgs = false;
  if (rssEnabled && prevRssUrl != rssURL) {
    rss.numRssTitles = 0;
    if (boardMode == MODE_RAIL || boardMode == MODE_TUBE) {
      prevProgressBarPosition=50;
      progressBar(F("Updating RSS headlines feed"),50);
      updateRssFeed();
    }
  }

  switch (boardMode) {
    case MODE_RAIL:
      altStationActive = setAlternateStation();
      // Create a cleaned platform filter (if any)
      raildata->cleanFilter(platformFilter,cleanPlatformFilter,sizeof(platformFilter));
      break;

    case MODE_TUBE:
      progressBar(F("Initialising TfL interface"),70);
      break;

    case MODE_BUS:
      progressBar(F("Initialising BusTimes interface"),70);
      // Create a cleaned filter
      busdata->cleanFilter(busFilter,cleanBusFilter,sizeof(busFilter));
      break;
  }
  station.numServices=0;
  messages.numMessages=0;
}

// WiFiManager callback, entered config mode
void wmConfigModeCallback (WiFiManager *myWiFiManager) {
  LOG_INFO("WiFiManager entered AP Configuration Mode. Displaying captive portal instructions.");
  showSetupScreen();
  wifiConfigured = true;
}

/*
 * Firmware / Web GUI Update functions
*/
bool isFirmwareUpdateAvailable() {
  int releaseMajor = ghUpdate.releaseId.substring(1,ghUpdate.releaseId.indexOf(".")).toInt();
  int releaseMinor = ghUpdate.releaseId.substring(ghUpdate.releaseId.indexOf(".")+1,ghUpdate.releaseId.indexOf("-")).toInt();
  if (VERSION_MAJOR > releaseMajor) return false;
  if ((VERSION_MAJOR == releaseMajor) && (VERSION_MINOR >= releaseMinor)) return false;
  return true;
}

// Callback function for displaying firmware update progress
void update_progress(int cur, int total) {
  int percent = ((cur * 100)/total);
  showFirmwareUpdateProgress(percent);
}

// Attempts to install newer firmware if available
bool checkForFirmwareUpdate() {
  LOG_WARN("Firmware Auto-Update is disabled for this custom fork.");
  return false;

  bool result = true;

  if (!isFirmwareUpdateAvailable()) return result;

  // Find the firmware binary in the release assets
  String updatePath="";
  for (int i=0;i<ghUpdate.releaseAssets;i++){
    if (ghUpdate.releaseAssetName[i] == "firmware.bin") {
      updatePath = ghUpdate.releaseAssetURL[i];
      break;
    }
  }
  if (updatePath.length()==0) {
    //  No firmware binary in release assets
    return result;
  }

  unsigned long tmr=millis()+1000;
  for (int i=30;i>=0;i--) {
    showFirmwareUpdateWarningScreen(ghUpdate.releaseDescription.c_str(),i);
    while (tmr>millis()) {
      yield();
      server.handleClient();
    }
    tmr=millis()+1000;
  }
  u8g2.clearDisplay();
  prevProgressBarPosition=0;
  showFirmwareUpdateProgress(0);  // So we don't have a blank screen
  WiFiClientSecure client;
  client.setInsecure();
  httpUpdate.onProgress(update_progress);
  httpUpdate.rebootOnUpdate(false); // Don't auto reboot, we'll handle it

  HTTPUpdateResult ret = httpUpdate.handleUpdate(client, updatePath, ghUpdate.accessToken);
  const char* msgTitle = "Firmware Update";
  switch (ret) {
    case HTTP_UPDATE_FAILED:
      char msg[60];
      sprintf(msg,"The update failed with error %d.",httpUpdate.getLastError());
      result=false;
      for (int i=20;i>=0;i--) {
        showUpdateCompleteScreen(msgTitle,msg,httpUpdate.getLastErrorString().c_str(),"",i,false);
        delay(1000);
      }
      break;

    case HTTP_UPDATE_NO_UPDATES:
      for (int i=10;i>=0;i--) {
        showUpdateCompleteScreen(msgTitle,"","No firmware updates were available.","",i,false);
        delay(1000);
      }
      break;

    case HTTP_UPDATE_OK:
      for (int i=20;i>=0;i--) {
        showUpdateCompleteScreen(msgTitle,"The firmware update has completed successfully.","For more information visit the URL below:","github.com/gadec-uk/departures-board/releases",i,true);
        delay(1000);
      }
      ESP.restart();
      break;
  }
  u8g2.clearDisplay();
  drawStartupHeading();
  u8g2.sendBuffer();
  return result;
}

/*
 * Station Board functions - pulling updates and animating the Departures Board main display
 */

void addRssMessage() {
    // Check if we need to add RSS headlines
    if (rssEnabled && messages.numMessages<MAXBOARDMESSAGES && rss.numRssTitles>0) {
      sprintf(messages.messages[messages.numMessages],"%s: %s",rssName.c_str(),rss.rssTitle[0]);
      for (int i=1;i<rss.numRssTitles;i++) {
        if (strlen(messages.messages[messages.numMessages]) + strlen(rss.rssTitle[i]) + 1 < MAXMESSAGESIZE) {
          strcat(messages.messages[messages.numMessages], (boardMode==MODE_TUBE)?"\x81":"\x90");
          strcat(messages.messages[messages.numMessages],rss.rssTitle[i]);
        } else {
          break;
        }
      }
      messages.numMessages++;
      rssAddedtoMsgs = true;
    }
}

/**
 * @brief Remove rss message
 */
void removeRssMessage() {
  if (rssAddedtoMsgs) {
    messages.numMessages--; // Remove the RSS entry so we don't confuse change detection
    rssAddedtoMsgs = false;
  }
}

/**
 * @brief Call the Rail API to refresh the National Rail board data
 * @return true if successful
 */
bool getStationBoard() {
  if (!firstLoad) showUpdateIcon(true);
  removeRssMessage();
  lastUpdateResult = raildata->updateDepartures(&station,&messages,crsCode,nrToken,MAXBOARDSERVICES,enableBus,callingCrsCode,cleanPlatformFilter,nrTimeOffset);
  nextDataUpdate = millis()+apiRefreshRate;
  if (lastUpdateResult == UPD_SUCCESS || lastUpdateResult == UPD_NO_CHANGE) {
    showUpdateIcon(false);
    lastDataLoadTime=millis();
    noDataLoaded=false;
    dataLoadSuccess++;
    addRssMessage();
    return true;
  } else if (lastUpdateResult == UPD_DATA_ERROR || lastUpdateResult == UPD_TIMEOUT) {
    lastLoadFailure=millis();
    dataLoadFailure++;
    nextDataUpdate = millis() + 30000; // 30 secs
    showUpdateIcon(false);
    return false;
  } else if (lastUpdateResult == UPD_UNAUTHORISED) {
    showTokenErrorScreen();
    while (true) { server.handleClient(); yield();}
  } else {
    showUpdateIcon(false);
    dataLoadFailure++;
    return false;
  }
}

/**
 * @brief Draw the main upcoming train departure in prominent font
 * @param showVia Toggle via destinations animation frame
 */
void drawPrimaryService(bool showVia) {
  int destPos;
  char clipDestination[MAXLOCATIONSIZE];
  char etd[16];

  u8g2.setFont(NatRailTall12);
  blankArea(0,LINE1,256,LINE2-LINE1);
  destPos = u8g2.drawStr(0,LINE1-1,station.service[0].sTime) + 6;
  if (station.service[0].platform[0] && strlen(station.service[0].platform)<3 && station.service[0].serviceType == TRAIN && !hidePlatform) {
    destPos += u8g2.drawStr(destPos,LINE1-1,station.service[0].platform) + 6;
  } else if (station.service[0].serviceType == BUS) {
    destPos += u8g2.drawStr(destPos,LINE1-1,"~") + 6; // Bus icon
  }
  if (isDigit(station.service[0].etd[0])) sprintf(etd,"Exp %s",station.service[0].etd);
  else strcpy(etd,station.service[0].etd);
  int etdWidth = getStringWidth(etd);
  u8g2.drawStr(SCREEN_WIDTH - etdWidth,LINE1-1,etd);
  // Space available for destination name
  int spaceAvailable = SCREEN_WIDTH - destPos - etdWidth - 6;
  if (showVia) strcpy(clipDestination,station.service[0].via);
  else strcpy(clipDestination,station.service[0].destination);
  if (getStringWidth(clipDestination) > spaceAvailable) {
    while (getStringWidth(clipDestination) > (spaceAvailable - 8)) {
      clipDestination[strlen(clipDestination)-1] = '\0';
    }
    // check if there's a trailing space left
    if (clipDestination[strlen(clipDestination)-1] == ' ') clipDestination[strlen(clipDestination)-1] = '\0';
    strcat(clipDestination,"...");
  }
  u8g2.drawStr(destPos,LINE1-1,clipDestination);
  // Set font back to standard
  u8g2.setFont(NatRailSmall9);
}

/**
 * @brief Draw secondary/tertiary train departures
 * @param line Index of the service to draw
 * @param y Y coordinate constraint for scrolling
 */
void drawServiceLine(int line, int y) {
  char clipDestination[30];
  char ordinal[5];

  switch (line) {
    case 1:
      strcpy(ordinal,"2nd ");
      break;
    case 2:
      strcpy(ordinal,"3rd ");
      break;
    default:
      sprintf(ordinal,"%dth ",line+1);
      break;
  }

  u8g2.setFont(NatRailSmall9);
  blankArea(0,y,256,9);

  if (line<station.numServices) {
    u8g2.drawStr(0,y-1,ordinal);
    int destPos = u8g2.drawStr(23,y-1,station.service[line].sTime) + 27;
    char plat[3];
    if (station.platformAvailable && !hidePlatform) {
      if (station.service[line].platform[0] && strlen(station.service[line].platform)<3 && station.service[line].serviceType == TRAIN) {
        strncpy(plat,station.service[line].platform,3);
        plat[2]='\0';
      } else {
        if (station.service[line].serviceType == BUS) strcpy(plat,"\x86");  // Bus icon
        else strcpy(plat,"\x96\x96");
      }
      u8g2.drawStr(destPos+11-getStringWidth(plat),y-1,plat);
      destPos+=16;
    }
    char etd[16];
    if (isDigit(station.service[line].etd[0])) sprintf(etd,"Exp %s",station.service[line].etd);
    else strcpy(etd,station.service[line].etd);
    int etdWidth = getStringWidth(etd);
    u8g2.drawStr(SCREEN_WIDTH - etdWidth,y-1,etd);
    // work out if we need to clip the destination
    strcpy(clipDestination,station.service[line].destination);

    int spaceAvailable = SCREEN_WIDTH - destPos - etdWidth - 6;

    if (getStringWidth(clipDestination) > spaceAvailable) {
      while (getStringWidth(clipDestination) > spaceAvailable - 17) {
        clipDestination[strlen(clipDestination)-1] = '\0';
      }
      // check if there's a trailing space left
      if (clipDestination[strlen(clipDestination)-1] == ' ') clipDestination[strlen(clipDestination)-1] = '\0';
      strcat(clipDestination,"...");
    }
    u8g2.drawStr(destPos,y-1,clipDestination);
  } else {
    if (weatherMsg[0] && line==station.numServices) {
      // We're showing the weather
      centreText(weatherMsg,y-1);
    } else {
      // We're showing the mandatory attribution
      centreText(nrAttributionn,y-1);
    }
  }
}

/**
 * @brief The main render function to assemble the National Rail board
 */
void drawStationBoard() {
  numMessages=0;
  if (firstLoad) {
    // Clear the entire screen for the first load since boot up/wake from sleep
    u8g2.clearBuffer();
    u8g2.setContrast(brightness);
    firstLoad=false;
    line3Service = noScrolling ? 1 : 0;
  } else {
    // Clear the top two lines
    blankArea(0,LINE0,256,LINE2-1);
  }
  drawStationHeader(station.location,callingStation,platformFilter,nrTimeOffset);

  // Draw the primary service line
  isShowingVia=false;
  viaTimer=millis()+300000;  // effectively don't check for via
  if (station.numServices) {
    drawPrimaryService(false);
    if (station.service[0].via[0]) viaTimer=millis()+4000;
    if (station.service[0].isCancelled) {
      // This train is cancelled
      if (station.serviceMessage[0]) {
        strcpy(line2[0],station.serviceMessage);
        numMessages=1;
      }
    } else {
      // The train is not cancelled
      if (station.service[0].isDelayed && station.serviceMessage[0]) {
        // The train is delayed and there's a reason
        strcpy(line2[0],station.serviceMessage);
        numMessages++;
      }
      if (station.calling[0]) {
        // Add the calling stops message
        sprintf(line2[numMessages],"Calling at: %s",station.calling);
        numMessages++;
      }
      if (strcmp(station.origin, station.location)==0) {
        // Service originates at this station
        if (station.service[0].opco[0]) {
          sprintf(line2[numMessages],"This %s service starts here.",station.service[0].opco);
        } else {
          strcpy(line2[numMessages],"This service starts here.");
        }
        // Add the seating if available
        switch (station.service[0].classesAvailable) {
          case 1:
            strcat(line2[numMessages],firstClassSeating);
            break;
          case 2:
            strcat(line2[numMessages],standardClassSeating);
            break;
          case 3:
            strcat(line2[numMessages],dualClassSeating);
            break;
        }
        numMessages++;
      } else {
        // Service originates elsewhere
        strcpy(line2[numMessages],"");
        if (station.service[0].opco[0]) {
          if (station.origin[0]) {
            sprintf(line2[numMessages],"This is the %s service from %s.",station.service[0].opco,station.origin);
          } else {
            sprintf(line2[numMessages],"This is the %s service.",station.service[0].opco);
          }
        } else {
          if (station.origin[0]) {
            sprintf(line2[numMessages],"This service originated at %s.",station.origin);
          }
        }
        // Add the seating if available
        switch (station.service[0].classesAvailable) {
          case 1:
            strcat(line2[numMessages],firstClassSeating);
            break;
          case 2:
            strcat(line2[numMessages],standardClassSeating);
            break;
          case 3:
            strcat(line2[numMessages],dualClassSeating);
            break;
        }
        if (line2[numMessages][0]) numMessages++;
      }
      if (station.service[0].trainLength) {
        // Add the number of carriages message
        sprintf(line2[numMessages],"This train is formed of %d coaches.",station.service[0].trainLength);
        numMessages++;
      }
    }
    // Add any nrcc messages
    for (int i=0;i<messages.numMessages;i++) {
      strcpy(line2[numMessages],messages.messages[i]);
      numMessages++;
    }
    // Setup for the first message to rollover to
    isScrollingStops=false;
    currentMessage=numMessages-1;
    if (noScrolling && station.numServices>1) {
      drawServiceLine(1,LINE2);
    }
  } else {
    blankArea(0,LINE2,256,LINE4-LINE2);
    u8g2.setFont(NatRailTall12);
    centreText(F("There are no scheduled services at this station."),LINE1-1);
    numMessages = messages.numMessages;
    for (int i=0;i<messages.numMessages;i++) {
      strcpy(line2[i],messages.messages[i]);
    }
    // Setup for the first message to rollover to
    isScrollingStops=false;
    currentMessage=numMessages-1;
  }
  u8g2.setFont(NatRailSmall9);
  u8g2.sendBuffer();
}

/*
 *
 * London Underground Board
 *
 */

/**
 * @brief Render the current time in the top right corner for TfL/Bus boards
 * @param update If true, visually refresh the display area immediately
 */
void drawCurrentTimeUG(bool update) {
  char sysTime[29];
  getLocalTime(&timeinfo);

  sprintf(sysTime,"%02d:%02d:%02d",timeinfo.tm_hour,timeinfo.tm_min,timeinfo.tm_sec);
  if (strcmp(displayedTime,sysTime)) {
    u8g2.setFont(UndergroundClock8);
    blankArea(99,ULINE4,58,8);
    u8g2.drawStr(99,ULINE4-1,sysTime);
    if (update) u8g2.updateDisplayArea(12,7,8,1);
    strcpy(displayedTime,sysTime);
    u8g2.setFont(Underground10);

    if (dateEnabled && timeinfo.tm_mday!=dateDay) {
      if (boardMode == MODE_TUBE) drawStationHeader(tubeName.c_str(),"","",0);
/**
 * @brief Draw station header
 * @param busName.c_str()
 * @param ""
 * @param busFilter
 * @param 0
 * @return Return value
 */
      else drawStationHeader(busName.c_str(),"",busFilter,0);
      if (update) u8g2.sendBuffer();  // Just refresh on new date
      u8g2.setFont(Underground10);
    }
  }
}

// Callback from the TfLdataClient/busDataClient library when processing data. Shows progress at startup and keeps clock running
void tflCallback() {
  if (firstLoad) {
    if (startupProgressPercent<95) {
      startupProgressPercent+=5;
      if (boardMode == MODE_TUBE) progressBar(F("Initialising TfL interface"),startupProgressPercent);
/**
 * @brief Progress bar
 * @param interface")
 * @param startupProgressPercent
 * @return Return value
 */
      else progressBar(F("Initialising BusTimes interface"),startupProgressPercent);
    }
  } else if (millis()>nextClockUpdate) {
    nextClockUpdate = millis()+500;
    drawCurrentTimeUG(true);
  }
}

/**
 * @brief Call the TfL API to refresh the Underground arrivals board data
 * @return true if successful
 */
bool getUndergroundBoard() {
  if (!firstLoad) showUpdateIcon(true);
  removeRssMessage();
  lastUpdateResult = tfldata->updateArrivals(&station,&messages,tubeId,tflAppkey,&tflCallback);
  nextDataUpdate = millis()+UGDATAUPDATEINTERVAL; // default update freq
  if (lastUpdateResult == UPD_SUCCESS || lastUpdateResult == UPD_NO_CHANGE) {
    showUpdateIcon(false);
    lastDataLoadTime=millis();
    noDataLoaded=false;
    dataLoadSuccess++;
    if (noScrolling) messages.numMessages = 0; else addRssMessage();
    return true;
  } else if (lastUpdateResult == UPD_DATA_ERROR || lastUpdateResult == UPD_TIMEOUT) {
    lastLoadFailure=millis();
    dataLoadFailure++;
    nextDataUpdate = millis() + 30000; // 30 secs
    showUpdateIcon(false);
    return false;
  } else if (lastUpdateResult == UPD_UNAUTHORISED) {
    showTokenErrorScreen();
    while (true) { server.handleClient(); yield();}
  } else {
    showUpdateIcon(false);
    dataLoadFailure++;
    return false;
  }
}

/**
 * @brief Draw a specific TfL Underground arrival line
 * @param serviceId Index of the service
 * @param y Y coordinate constraint
 */
void drawUndergroundService(int serviceId, int y) {
  char serviceData[8+MAXLINESIZE+MAXLOCATIONSIZE];

  u8g2.setFont(Underground10);
  blankArea(0,y,256,10);

  if (serviceId < station.numServices) {
    sprintf(serviceData,"%d %s",serviceId+1,station.service[serviceId].destination);
    u8g2.drawStr(0,y-1,serviceData);
    if (serviceId || station.service[serviceId].timeToStation > 30) {
      if (station.service[serviceId].timeToStation <= 60) u8g2.drawStr(SCREEN_WIDTH-19,y-1,"Due");
      else {
        int mins = (station.service[serviceId].timeToStation + 30) / 60; // Round to nearest minute
        sprintf(serviceData,"%d",mins);
        if (mins==1) u8g2.drawStr(SCREEN_WIDTH-22,y-1,"min"); else u8g2.drawStr(SCREEN_WIDTH-22,y-1,"mins");
        u8g2.drawStr(SCREEN_WIDTH-27-(strlen(serviceData)*7),y-1,serviceData);
      }
    }
  }
}

/**
 * @brief The main render function to assemble the TfL Underground board
 */
void drawUndergroundBoard() {
  numMessages = messages.numMessages;
  if (line3Service==0) line3Service=1;
  attributionScrolled=false;
  if (firstLoad) {
    // Clear the entire screen for the first load since boot up/wake from sleep
    u8g2.clearBuffer();
    u8g2.setContrast(brightness);
    firstLoad=false;
  } else {
      // Clear the top three lines
      blankArea(0,ULINE0,256,ULINE3-1);
  }
  drawStationHeader(tubeName.c_str(),"","",0);

  if (station.boardChanged) {
    // prepare to scroll up primary services
    scrollPrimaryYpos = 11;
    isScrollingPrimary = true;
    // reset line3
    line3Service = 99;
    prevScrollStopsLength = 0;
    currentMessage=99;
    blankArea(0,ULINE3,256,11);
    serviceTimer=0;
  } else {
    // Draw the primary service line(s)
    if (station.numServices) {
      drawUndergroundService(0,ULINE1);
      if (station.numServices>1) drawUndergroundService(1,ULINE2);
    } else {
      u8g2.setFont(Underground10);
      centreText(F("There are no scheduled arrivals at this station."),ULINE1-1);
    }
  }
  for (int i=0;i<messages.numMessages;i++) {
    strcpy(line2[i],messages.messages[i]);
  }
  // Add attribution msg
  strcpy(line2[messages.numMessages],tflAttribution);
  messages.numMessages++;

  u8g2.sendBuffer();
}

/*
 *
 * Bus Departures Board
 *
 */
/**
 * @brief Call the bustimes.org API to refresh the Bus board data
 * @return true if successful
 */
bool getBusDeparturesBoard() {
  if (!firstLoad) showUpdateIcon(true);
  lastUpdateResult = busdata->updateDepartures(&station,busAtco,cleanBusFilter,&tflCallback);
  nextDataUpdate = millis()+BUSDATAUPDATEINTERVAL; // default update freq
  if (lastUpdateResult == UPD_SUCCESS || lastUpdateResult == UPD_NO_CHANGE) {
    showUpdateIcon(false);
    lastDataLoadTime=millis();
    noDataLoaded=false;
    dataLoadSuccess++;
    // Work out the max column size for service numbers
    busDestX=0;
    u8g2.setFont(NatRailSmall9);
    for (int i=0;i<station.numServices;i++) {
      int svcWidth = getStringWidth(station.service[i].via);
      busDestX = (busDestX > svcWidth) ? busDestX : svcWidth;
    }
    busDestX+=5;
    return true;
  } else if (lastUpdateResult == UPD_DATA_ERROR || lastUpdateResult == UPD_TIMEOUT) {
    lastLoadFailure=millis();
    dataLoadFailure++;
    nextDataUpdate = millis() + 30000; // 30 secs
    showUpdateIcon(false);
    return false;
  } else if (lastUpdateResult == UPD_UNAUTHORISED) {
    showTokenErrorScreen();
    while (true) { server.handleClient(); yield();}
  } else {
    showUpdateIcon(false);
    dataLoadFailure++;
    return false;
  }
}

/**
 * @brief Draw a specific Bus arrival line
 * @param serviceId Index of the bus service
 * @param y Y coordinate constraint
 * @param destPos Right aligned destination text X offset
 */
void drawBusService(int serviceId, int y, int destPos) {
  char clipDestination[MAXLOCATIONSIZE];
  char etd[16];

  if (serviceId < station.numServices) {
    u8g2.setFont(NatRailSmall9);
    blankArea(0,y,256,9);

    u8g2.drawStr(0,y-1,station.service[serviceId].via);
    int etdWidth = 25;
    if (isDigit(station.service[serviceId].etd[0])) {
      sprintf(etd,"Exp %s",station.service[serviceId].etd);
      etdWidth = 47;
    } else strcpy(etd,station.service[serviceId].sTime);
    u8g2.drawStr(SCREEN_WIDTH - etdWidth,y-1,etd);

    // work out if we need to clip the destination
    strcpy(clipDestination,station.service[serviceId].destination);
    int spaceAvailable = SCREEN_WIDTH - destPos - etdWidth - 6;
    if (getStringWidth(clipDestination) > spaceAvailable) {
      while (getStringWidth(clipDestination) > spaceAvailable - 17) {
        clipDestination[strlen(clipDestination)-1] = '\0';
      }
      // check if there's a trailing space left
      if (clipDestination[strlen(clipDestination)-1] == ' ') clipDestination[strlen(clipDestination)-1] = '\0';
      strcat(clipDestination,"...");
    }
    u8g2.drawStr(destPos,y-1,clipDestination);
  }
}

/**
 * @brief The main render function to assemble the Bus board
 */
void drawBusDeparturesBoard() {

  if (line3Service==0) line3Service=1;
  if (firstLoad) {
    // Clear the entire screen for the first load since boot up/wake from sleep
    u8g2.clearBuffer();
    u8g2.setContrast(brightness);
    firstLoad=false;
  } else {
      // Clear the top three lines
      blankArea(0,ULINE0,256,ULINE3-1);
  }
  drawStationHeader(busName.c_str(),"",busFilter,0);

  if (station.boardChanged) {
    // prepare to scroll up primary services
    scrollPrimaryYpos = 11;
    isScrollingPrimary = true;
    // reset line3
    if (station.numServices>2) {
      line3Service=2;
    } else {
      line3Service=99;
    }
    currentMessage = -1;
    blankArea(0,ULINE3,256,11);
    serviceTimer=0;
  } else {
    // Draw the primary service line(s)
    if (station.numServices) {
      drawBusService(0,ULINE1,busDestX);
      if (station.numServices>1) drawBusService(1,ULINE2,busDestX);
    } else {
      u8g2.setFont(NatRailSmall9);
      centreText(F("There are no scheduled services at this stop."),ULINE1-1);
    }
  }
  messages.numMessages=0;
  if (weatherEnabled && weatherMsg[0]) {
    strcpy(line2[messages.numMessages++],weatherMsg);
  }
  strcpy(line2[messages.numMessages++],btAttribution);
  u8g2.sendBuffer();
}

