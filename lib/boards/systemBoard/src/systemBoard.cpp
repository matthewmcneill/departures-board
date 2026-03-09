/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/boards/systemBoard/src/systemBoard.cpp
 * Description: Implementations of standalone full-screen overlays (splash, setup wizards,
 *              various error screens, and screensavers).
 */

#include "../include/systemBoard.hpp"
#include <configManager.hpp>
#include <timeManager.hpp>
#include <timeWidgets.hpp>
#include <displayManager.hpp>
#include "../../nationalRailBoard/include/nationalRailBoard.hpp"
#include "../../interfaces/IStation.hpp"
#include <rssClient.h>
#include <weatherClient.h>
#include <Logger.hpp>

extern DisplayManager displayManager;
extern stnMessages messages;
extern rssClient* rss;
extern NationalRailBoard nationalRailBoard;
extern unsigned long nextClockUpdate;

bool wifiConfigured = false;
bool wifiConnected = false;
unsigned long lastWiFiReconnect = 0;
bool firstLoad = true;
int startupProgressPercent = 0;
int prevProgressBarPosition = 0;

bool getWifiConfigured() { return wifiConfigured; }
void setWifiConfigured(bool configured) { wifiConfigured = configured; }

bool getWifiConnected() { return wifiConnected; }
void setWifiConnected(bool connected) { wifiConnected = connected; }

unsigned long getLastWiFiReconnect() { return lastWiFiReconnect; }
void setLastWiFiReconnect(unsigned long time) { lastWiFiReconnect = time; }

bool getFirstLoad() { return firstLoad; }
void setFirstLoad(bool load) { firstLoad = load; }

int getPrevProgressBarPosition() { return prevProgressBarPosition; }
void setPrevProgressBarPosition(int pos) { prevProgressBarPosition = pos; }

int getStartupProgressPercent() { return startupProgressPercent; }
void setStartupProgressPercent(int percent) { startupProgressPercent = percent; }
extern unsigned long nextDataUpdate;
extern bool noDataLoaded;

/**
 * @brief Show setup keys help screen
 */
void showSetupKeysHelpScreen() {
  u8g2.setFont(NatRailSmall9);
  u8g2.clearBuffer();
  centreText("First Boot Keys Help",3);
  drawTruncatedText("Check National Rail",14);
  drawTruncatedText("Website -> My Account ->",24);
  drawTruncatedText("OpenLDBWS -> get token.",34);
  drawTruncatedText("TfL: API Portal -> keys.",44);
  drawTruncatedText("Connect to config AP.",54);
  u8g2.sendBuffer();
}

/**
 * @brief Show setup CRS help screen
 */
void showSetupCrsHelpScreen() {
  u8g2.setFont(NatRailSmall9);
  u8g2.clearBuffer();
  centreText("CRS Help",3);
  drawTruncatedText("National Rail specific.",14);
  drawTruncatedText("This is the 3 letter",24);
  drawTruncatedText("code for a station.",34);
  drawTruncatedText("Eg EUS=Euston VIC=Victoria",44);
  drawTruncatedText("Connect to config AP.",54);
  u8g2.sendBuffer();
}

/**
 * @brief Render the initial configuration wizard instructions via WiFi
 */
void showSetupScreen(IPAddress ip) {
  static unsigned long lastInteraction = 0;
  static int screenState = 0;
  char address[26];
  sprintf(address,"%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);
  u8g2.setFont(NatRailSmall9);

  if (millis()-lastInteraction>8000) {
    if (++screenState>2) screenState=0;
    lastInteraction=millis();
    if (screenState==1) showSetupKeysHelpScreen();
    else if (screenState==2) showSetupCrsHelpScreen();
    else {
      u8g2.clearBuffer();
      centreText(F("Wifi Setup Mode"),3);
      drawTruncatedText("Connect to Wifi:",14);
      drawTruncatedText("Departures-Board",24);
      drawTruncatedText("Point browser to:",34);
      drawTruncatedText(address,44);
      drawTruncatedText("(Hold D3 for help)",54);
      u8g2.sendBuffer();
    }
  }
}

/**
 * @brief Render an error screen when no data could be fetched from the API
 */
void showNoDataScreen() {
  u8g2.clearBuffer();
  u8g2.setFont(NatRailSmall9);
  centreText(F("** COMMS ERROR **"),3);
  u8g2.setFont(NatRailTall12);
  centreText(F("NO DEPARTURE DATA"),20);
  u8g2.setFont(NatRailSmall9);
  centreText(F("UNABLE TO GET DATA FOR"),38);
  centreText(F("STATION."),48);
  u8g2.sendBuffer();  
}

/**
 * @brief Render a critical error if National Rail WSDL fails to init
 */
void showWsdlFailureScreen() {
  u8g2.clearBuffer();
  u8g2.setFont(NatRailSmall9);
  centreText(F("** WSDL ERROR **"),3);
  u8g2.setFont(NatRailTall12);
  centreText(F("FAILED TO INIT API"),20);
  u8g2.setFont(NatRailSmall9);
  centreText(F("CANNOT CONTACT NATIONAL"),38);
  centreText(F("RAIL SERVERS."),48);
  u8g2.sendBuffer();  
}

/**
 * @brief Render an error meaning the National Rail or TfL token is invalid
 */
void showTokenErrorScreen() {
  u8g2.clearBuffer();
  u8g2.setFont(NatRailSmall9);
  centreText(F("** AUTHORISATION **"),3);
  u8g2.setFont(NatRailTall12);
  centreText(F("INVALID API KEY"),20);
  u8g2.setFont(NatRailSmall9);
  centreText(F("CHECK YOUR TOKEN AND"),38);
  centreText(F("TFL CREDENTIALS VIA"),48);
  centreText(F("THE WEB SETUP WIZARD."),58);
  u8g2.sendBuffer();  
}

/**
 * @brief Render a CRS error screen
 */
void showCRSErrorScreen() {
  u8g2.clearBuffer();
  u8g2.setFont(NatRailSmall9);
  centreText(F("** STATION ERROR **"),3);
  u8g2.setFont(NatRailTall12);
  centreText(F("INVALID CRS/NAPTAN"),20);
  u8g2.setFont(NatRailSmall9);
  centreText(F("STATION CODE NOT"),38);
  centreText(F("RECOGNISED BY API."),48);
  u8g2.sendBuffer();  
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




/* ==========[ FIRMWARE UPDATE UI ]========== */


/**
 * @brief Show or hide the OTA update available icon
 * @param show true to show, false to clear
 */
void showFirmwareUpdateIcon(bool show) {
  if (show) {
    u8g2.setFont(NatRailTall12);
    u8g2.drawStr(0,50,"}");
    u8g2.setFont(NatRailSmall9);
  } else {
    blankArea(0,50,6,13);
  }
  u8g2.updateDisplayArea(0,6,1,2);
}

/**
 * @brief Show firmware update warning screen
 * @param msg The version name fetched from Github
 * @param secs Seconds until auto-reboot and flash
 */
void showFirmwareUpdateWarningScreen(const char *msg, int secs) {
  char countdown[60];
  u8g2.clearBuffer();
  u8g2.setFont(NatRailTall12);
  centreText("Firmware Update Available",-1);
  u8g2.setFont(NatRailSmall9);
  centreText("A new version of the Departures Board firmware",14);
  sprintf(countdown,"will be installed in %d seconds. This provides:",secs);
  centreText(countdown,26);
  sprintf(countdown,"\"%s\"",msg);
  centreText(countdown,40);
  centreText("* DO NOT REMOVE THE POWER DURING THE UPDATE *",54);
  u8g2.sendBuffer();
}

/**
 * @brief Show firmware update progress bar during flash memory write
 * @param percent 0-100 indicating download progress
 */
void showFirmwareUpdateProgress(int percent) {
  u8g2.setFont(NatRailSmall9);
  char msg[10];
  sprintf(msg,"%d%%",percent);
  blankArea(0,24,256,25);
  centreText(msg,24);
  drawProgressBar(percent);
  u8g2.sendBuffer();
}

/**
 * @brief Show update complete screen
 */
void showFirmwareUpdateCompleteScreen(const char *title, const char *msg1, const char *msg2, const char *msg3, int secs, bool showReboot) {
  char countdown[60];
  u8g2.clearBuffer();
  u8g2.setFont(NatRailTall12);
  centreText(title,-1);
  u8g2.setFont(NatRailSmall9);
  centreText(msg1,14);
  centreText(msg2,26);
  centreText(msg3,40);
  if (showReboot) {
    sprintf(countdown,"The board will restart in %d seconds...",secs);
    centreText(countdown,54);
  }
  u8g2.sendBuffer();
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

// Check if the NR clock needs to be updated
void doClockCheck() {
  if (!firstLoad) {
    if (millis()>nextClockUpdate) {
      drawCurrentTimeUG(true);
      nextClockUpdate=millis()+250;
    }
  }
}



// Returns true if an alternate station is enabled and we're within the activation period
bool isAltActive() {
  if (!nationalRailBoard.getAltStationEnabled()) return false;
  getLocalTime(&timeinfo);
  byte myHour = timeinfo.tm_hour;
  if (nationalRailBoard.getAltStarts() > nationalRailBoard.getAltEnds()) {
    if ((myHour >= nationalRailBoard.getAltStarts()) || (myHour < nationalRailBoard.getAltEnds())) return true; else return false;
  } else {
    if ((myHour >= nationalRailBoard.getAltStarts()) && (myHour < nationalRailBoard.getAltEnds())) return true; else return false;
  }
  return false;
}

// Callback from the raildataXMLclient library when processing data. As this can take some time, this callback is used to keep the clock working
// and to provide progress on the initial load at boot
void raildataCallback(int stage, int nServices) {
  if (firstLoad) {
    // Note: Assuming NR_MAX_SERVICES is ~40 -> approx 20 * 40
    int percent = ((nServices*20)/40)+80;
    progressBar(F("Initialising National Rail interface"),percent);
  } else doClockCheck();
}

// Stores/updates the url of our Web GUI
void updateMyUrl() {
  IPAddress ip = WiFi.localIP();
  snprintf(myUrl,sizeof(myUrl),"http://%u.%u.%u.%u",ip[0],ip[1],ip[2],ip[3]);
}

/**
 * @brief Check schedule and set alternate station variables if in time range
 * @return true if alternate station is currently active
 */
bool setAlternateStation() {
  if (boardMode==MODE_RAIL && nationalRailBoard.getAltStationEnabled() && isAltActive()) {
    // Switch to alternate station
    nationalRailBoard.setCrsCode(nationalRailBoard.getAltCrsCode());
    nationalRailBoard.setStationLat(nationalRailBoard.getAltLat());
    nationalRailBoard.setStationLon(nationalRailBoard.getAltLon());
    nationalRailBoard.setCallingCrsCode(nationalRailBoard.getAltCallingCrsCode());
    nationalRailBoard.setCallingStation(nationalRailBoard.getAltCallingStation());
    nationalRailBoard.setPlatformFilter(nationalRailBoard.getAltPlatformFilter());
    return true;
  } else {
    return false;
  }
  return false;
}

/**
 * @brief Update rss feed
 */
void updateRssFeed() {
  int res = rss->loadFeed(rss->getRssURL());
  rss->setLastRssUpdateResult(res);
  if (res == UPD_SUCCESS) rss->setNextRssUpdate(millis() + 600000); // update every ten minutes
  else rss->setNextRssUpdate(millis() + 300000); // Failed so try again in 5 minutes
}

/**
 * @brief Perform a gentle software reset to switch stations without full reboot
 */
void softResetBoard() {
  int previousMode = boardMode;
  String prevRssUrl = String(rss->getRssURL());

  // Reload the settings
  loadConfig();
  if (displayManager.getFlipScreen()) u8g2.setFlipMode(1); else u8g2.setFlipMode(0);
  if (timeManager.getTimezone()!="") {
    setenv("TZ",timeManager.getTimezone().c_str(),1);
  } else {
    setenv("TZ",TimeManager::ukTimezone,1);
  }
  tzset();
  u8g2.clearBuffer();
  drawStartupHeading();
  u8g2.updateDisplay();

  // Force an update asap
  nextDataUpdate = 0;
  currentWeather->setNextWeatherUpdate(0);
  displayManager.resetState();
  firstLoad=true;
  noDataLoaded=true;
  prevProgressBarPosition=70;
  startupProgressPercent=70;
  // If weather not enabled, clear msg? Weather disabled msg handled elsewhere
  if (previousMode!=boardMode) {
    // Stage 3 multi-board logic initialized here.
  }

  rss->setRssAddedtoMsgs(false);
  if (rss->getRssEnabled() && prevRssUrl != rss->getRssURL()) {
    rss->numRssTitles = 0;
    if (boardMode == MODE_RAIL || boardMode == MODE_TUBE) {
      prevProgressBarPosition=50;
      progressBar(F("Updating RSS headlines feed"),50);
      updateRssFeed();
    }
  }

  if (boardMode == MODE_RAIL) {
      nationalRailBoard.setAltStationActive(setAlternateStation());
  } else if (boardMode == MODE_TUBE) {
      progressBar(F("Initialising TfL interface"),70);
  } else if (boardMode == MODE_BUS) {
      progressBar(F("Initialising BusTimes interface"),70);
  }
  messages.numMessages=0;
}

// WiFiManager callback, entered config mode
void wmConfigModeCallback(WiFiManager *myWiFiManager) {
  LOG_INFO("WiFiManager entered AP Configuration Mode. Displaying captive portal instructions.");
  showSetupScreen(WiFi.softAPIP());
  wifiConfigured = true;
}

void addRssMessage() {
    // Check if we need to add RSS headlines
    if (rss->getRssEnabled() && messages.numMessages<MAXBOARDMESSAGES && rss->numRssTitles>0) {
      sprintf(messages.messages[messages.numMessages],"%s: %s",rss->getRssName(),rss->rssTitle[0]);
      for (int i=1;i<rss->numRssTitles;i++) {
        if (strlen(messages.messages[messages.numMessages]) + strlen(rss->rssTitle[i]) + 1 < MAXMESSAGESIZE) {
          strcat(messages.messages[messages.numMessages], (boardMode==MODE_TUBE)?"\x81":"\x90");
          strcat(messages.messages[messages.numMessages],rss->rssTitle[i]);
        } else {
          break;
        }
      }
      messages.numMessages++;
      rss->setRssAddedtoMsgs(true);
    }
}

/**
 * @brief Remove rss message
 */
void removeRssMessage() {
  if (rss->getRssAddedtoMsgs()) {
    messages.numMessages--; // Remove the RSS entry so we don't confuse change detection
    rss->setRssAddedtoMsgs(false);
  }
}

// Callback from the TfLdataClient/busDataClient library when processing data. Shows progress at startup and keeps clock running
void tflCallback() {
  if (firstLoad) {
    if (startupProgressPercent<95) {
      startupProgressPercent+=5;
      if (boardMode == MODE_TUBE) progressBar(F("Initialising TfL interface"),startupProgressPercent);
      else progressBar(F("Initialising BusTimes interface"),startupProgressPercent);
    }
  } else if (millis()>nextClockUpdate) {
    nextClockUpdate = millis()+500;
    drawCurrentTimeUG(true);
  }
}
