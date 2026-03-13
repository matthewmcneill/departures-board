/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/boards/systemBoard/systemBoard.cpp
 * Description: Implementation of standalone full-screen overlays, error screens,
 *              and system control functions.
 *
 * Exported Functions/Classes:
 * - getSystemBoardRegistry: Retrieve a configured system board singleton.
 * - drawStartupHeading: Renders boot logo and versioning.
 * - softResetBoard: Resets system logic without hardware reboot.
 * - tflCallback: Update callback for TfL/Bus data loads.
 * - showFirmwareUpdateIcon: Toggle the OTA notification icon.
 * - setAlternateStation: Apply time-based station overrides.
 */

#include "systemBoard.hpp"
#include <configManager.hpp>
#include <timeManager.hpp>
#include <displayManager.hpp>
#include "../nationalRailBoard/nationalRailBoard.hpp"
#include "../interfaces/iDisplayBoard.hpp"
#include "messageBoard.hpp"
#include "wizardBoard.hpp"
#include "helpBoard.hpp"
#include "loadingBoard.hpp"
#include "sleepingBoard.hpp"
#include "splashBoard.hpp"
#include "firmwareUpdateBoard.hpp"
#include <WiFiManager.h>
#include <rssClient.h>
#include <weatherClient.h>
#include <Logger.hpp>

// --- External Object Linkage ---
extern char myUrl[24];
extern ConfigManager configManager;
extern DisplayManager displayManager;      // Global display orchestrator
extern ConfigManager configManager;        // Central configuration store
extern stnMessages messages;           // Shared station message pool
extern rssClient* rss;                     // RSS headlines service

extern unsigned long nextDataUpdate;       // Calculated next poll timestamp
extern bool noDataLoaded;                  // Flag for valid payload state

// --- Module State ---
bool wifiConfigured = false;          // True if WiFiManager has credentials
bool wifiConnected = false;           // True if active WiFi connection established
unsigned long lastWiFiReconnect = 0;  // Last attempt timestamp (ms)
bool firstLoad = true;                // True during initial boot sequence
int startupProgressPercent = 0;       // Aggregated boot progress (0-100)
int prevProgressBarPosition = 0;      // Cached UI value to avoid redraw flicker



/**
 * @brief Get the Build Timestamp of the running firmware
 * @return String formatted build timestamp.
 */
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
 * @brief Check if the display animation ticks need yielding.
 */
void doClockCheck() {
  if (!firstLoad) {
    displayManager.yieldAnimationUpdate();
  }
}

/**
 * @brief Returns true if an alternate station is enabled and current time is within activation period.
 * @return bool True if alternate station should be displayed.
 */
bool isAltActive() {
  const Config& config = configManager.getConfig();
  if (!config.altStationEnabled) return false;
  
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return false;

  byte myHour = timeinfo.tm_hour;
  if (config.altStarts > config.altEnds) {
    if ((myHour >= config.altStarts) || (myHour < config.altEnds)) return true; else return false;
  } else {
    if ((myHour >= config.altStarts) && (myHour < config.altEnds)) return true; else return false;
  }
  return false;
}

/**
 * @brief Callback from National Rail data client to keep UI responsive during parsing.
 * @param stage Current processing stage.
 * @param nServices Number of services processed.
 */
void raildataCallback(int stage, int nServices) {
  if (firstLoad) {
    int percent = ((nServices*20)/40)+80;
    LoadingBoard* load = (LoadingBoard*)displayManager.getSystemBoard(SystemBoardId::SYS_BOOT_LOADING);
    load->setProgress("Initialising National Rail interface", percent);
    displayManager.showBoard(load);
  } else {
    displayManager.yieldAnimationUpdate();
  }
}

/**
 * @brief Update the internal URL string for the Web GUI.
 */
void updateMyUrl() {
  IPAddress ip = WiFi.localIP();
  snprintf(myUrl,sizeof(myUrl),"http://%u.%u.%u.%u",ip[0],ip[1],ip[2],ip[3]);
}

/**
 * @brief Check schedule and set alternate station variables if in time range.
 * @return true if alternate station is currently active.
 */
bool setAlternateStation() {
  const Config& config = configManager.getConfig();
  if (config.boardMode == MODE_RAIL && config.altStationEnabled && isAltActive()) {
    NationalRailBoard* nrb = (NationalRailBoard*)displayManager.getDisplayBoard(0);
    if (!nrb) return false;
    nrb->setCrsCode(config.altCrsCode);
    nrb->setStationLat(config.altLat);
    nrb->setStationLon(config.altLon);
    nrb->setCallingCrsCode(config.altCallingCrsCode);
    nrb->setCallingStation(config.altCallingStation);
    nrb->setPlatformFilter(config.altPlatformFilter);
    return true;
  }
  return false;
}

/**
 * @brief Trigger a manual update of the RSS feed headlines.
 */
void updateRssFeed() {
  int res = rss->loadFeed(rss->getRssURL());
  rss->setLastRssUpdateResult(res);
  if (res == UPD_SUCCESS) rss->setNextRssUpdate(millis() + 600000); 
  else rss->setNextRssUpdate(millis() + 300000);
}

/**
 * @brief Performs a soft reload of the application state based on new configuration.
 *        Avoids a full hardware reboot by resetting managers and buffers in memory.
 */
void softResetBoard() {
  const Config& config = configManager.getConfig();
  String prevRssUrl = String(rss->getRssURL());
 
  // --- Step 1: Reload Configuration and Synchronize Environment ---
  configManager.loadConfig();
  displayManager.applyConfig(config);
  
  if (timeManager.getTimezone()!="") {
    setenv("TZ",timeManager.getTimezone().c_str(),1);
  } else {
    setenv("TZ",TimeManager::ukTimezone,1);
  }
  tzset();
 
  // --- Step 2: Re-initialize Manager States and Timers ---
  SplashBoard* splash = (SplashBoard*)displayManager.getSystemBoard(SystemBoardId::SYS_BOOT_SPLASH);
  displayManager.showBoard(splash);
 
  nextDataUpdate = 0;
  currentWeather->setNextWeatherUpdate(0);
  displayManager.resetState();
  firstLoad=true;
  noDataLoaded=true;
  prevProgressBarPosition=70;
  startupProgressPercent=70;
 
  // --- Step 3: Service-specific Refresh ---
  rss->setRssAddedtoMsgs(false);
  if (rss->getRssEnabled() && prevRssUrl != rss->getRssURL()) {
    rss->numRssTitles = 0;
    if (config.boardMode == MODE_RAIL || config.boardMode == MODE_TUBE) {
      prevProgressBarPosition=50;
      LoadingBoard* load = (LoadingBoard*)displayManager.getSystemBoard(SystemBoardId::SYS_BOOT_LOADING);
      load->setProgress("Updating RSS headlines feed", 50);
      displayManager.showBoard(load);
      updateRssFeed();
    }
  }
 
  // --- Step 4: Finalize Interface Mode ---
  if (config.boardMode == MODE_RAIL) {
      NationalRailBoard* nrb = (NationalRailBoard*)displayManager.getDisplayBoard(0);
      if (nrb) nrb->setAltStationActive(setAlternateStation());
  } else if (config.boardMode == MODE_TUBE) {
      LoadingBoard* load = (LoadingBoard*)displayManager.getSystemBoard(SystemBoardId::SYS_BOOT_LOADING);
      load->setProgress("Initialising TfL interface", 70);
      displayManager.showBoard(load);
  } else if (config.boardMode == MODE_BUS) {
      LoadingBoard* load = (LoadingBoard*)displayManager.getSystemBoard(SystemBoardId::SYS_BOOT_LOADING);
      load->setProgress("Initialising BusTimes interface", 70);
      displayManager.showBoard(load);
  }
  messages.numMessages=0;
}

/**
 * @brief WiFiManager callback triggered when entering Configuration Access Point mode.
 * @param myWiFiManager Pointer to the active WiFiManager instance.
 */
void wmConfigModeCallback(WiFiManager *myWiFiManager) {
  LOG_INFO("WiFiManager entered AP Configuration Mode.");
  displayManager.showBoard(displayManager.getSystemBoard(SystemBoardId::SYS_WIFI_WIZARD));
  wifiConfigured = true;
}

/**
 * @brief Append RSS headlines to the scrolling message pool if enabled.
 */
void addRssMessage() {
    const Config& config = configManager.getConfig();
    if (rss->getRssEnabled() && messages.numMessages<MAXBOARDMESSAGES && rss->numRssTitles>0) {
      sprintf(messages.messages[messages.numMessages],"%s: %s",rss->getRssName(),rss->rssTitle[0]);
      for (int i=1;i<rss->numRssTitles;i++) {
        if (strlen(messages.messages[messages.numMessages]) + strlen(rss->rssTitle[i]) + 1 < MAXMESSAGESIZE) {
          strcat(messages.messages[messages.numMessages], (config.boardMode == MODE_TUBE)?"\x81":"\x90");
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
 * @brief Clean up RSS messages from the board message pool.
 */
void removeRssMessage() {
  if (rss->getRssAddedtoMsgs()) {
    messages.numMessages--;
    rss->setRssAddedtoMsgs(false);
  }
}

/**
 * @brief Update callback for TfL/Bus data loads to keep UI animated.
 */
void tflCallback() {
  const Config& config = configManager.getConfig();
  if (firstLoad) {
    if (startupProgressPercent < 95) {
      startupProgressPercent += 5;
      LoadingBoard* load = (LoadingBoard*)displayManager.getSystemBoard(SystemBoardId::SYS_BOOT_LOADING);
      if (load) {
        load->setHeading("Departures Board");
        load->setBuildTime(getBuildTime().c_str());
        if (config.boardMode == MODE_TUBE) load->setProgress("Initialising TfL interface", startupProgressPercent);
        else load->setProgress("Initialising BusTimes interface", startupProgressPercent);
        displayManager.showBoard(load);
        displayManager.resetState();
      }
    }
  } else {
    displayManager.yieldAnimationUpdate();
  }
}

// --- Accessors for System State ---

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
