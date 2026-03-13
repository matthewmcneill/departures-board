/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/systemManager/systemManager.cpp
 * Description: Implementation of the systemManager for global state and logic.
 */

#include "systemManager.hpp"
#include "appContext.hpp"
#include <Logger.hpp>
#include <timeManager.hpp>
#include <boards/nationalRailBoard/nationalRailBoard.hpp>
#include <boards/systemBoard/loadingBoard.hpp>
#include <boards/systemBoard/splashBoard.hpp>

/**
 * @brief Initialize default system state.
 */
systemManager::systemManager() 
    : context(nullptr), wifiConfigured(false), wifiConnected(false), 
      lastWiFiReconnect(0), firstLoad(true), startupProgressPercent(0), 
      prevProgressBarPosition(0), nextDataUpdate(0), lastDataLoadTime(0), 
      noDataLoaded(true), dataLoadSuccess(0), dataLoadFailure(0), 
      lastLoadFailure(0), lastUpdateResult(0) {
    strlcpy(myUrl, "http://0.0.0.0", sizeof(myUrl));
}

/**
 * @brief Link to the parent context and perform initial state sync.
 * @param contextPtr Pointer to the parent appContext.
 */
void systemManager::begin(appContext* contextPtr) {
    context = contextPtr;
    LOG_INFO("SYSTEM", "systemManager initialized.");
}

/**
 * @brief Periodic maintenance logic for application-wide state.
 */
void systemManager::tick() {
    // Connection Management
    if (WiFi.status() != WL_CONNECTED && wifiConnected) {
        wifiConnected = false;
        LOG_WARN("SYSTEM", "WiFi connection lost.");
    } else if (WiFi.status() == WL_CONNECTED && !wifiConnected) {
        wifiConnected = true;
        LOG_INFO("SYSTEM", "WiFi connected. IP: " + WiFi.localIP().toString());
        updateMyUrl();
    }

    if (WiFi.status() != WL_CONNECTED && millis() > lastWiFiReconnect + 10000) {
        LOG_INFO("SYSTEM", "Attempting WiFi reconnection...");
        WiFi.disconnect();
        delay(100);
        WiFi.reconnect();
        lastWiFiReconnect = millis();
    }

    DisplayManager& displayMgr = context->getDisplayManager();
    const Config& config = context->getConfigManager().getConfig();

    // Data Fetching Logic
    if (!displayMgr.getIsSleeping()) {
        if (millis() > nextDataUpdate && wifiConnected) {
            displayMgr.setOtaUpdateAvailable(true);
            
            iDisplayBoard* activeBoard = displayMgr.getDisplayBoard(displayMgr.getActiveSlotIndex());
            if (activeBoard) {
                LOG_INFO("DATA", "Triggering active board data update for slot index: " + String(displayMgr.getActiveSlotIndex()));
                lastUpdateResult = activeBoard->updateData();
            } else {
                LOG_WARN("DATA", "Active board is NULL for slot index: " + String(displayMgr.getActiveSlotIndex()));
            }

            // Select next update interval
            if (config.boardMode == 0)      nextDataUpdate = millis() + config.apiRefreshRate; // MODE_RAIL
            else if (config.boardMode == 1) nextDataUpdate = millis() + 30000; // MODE_TUBE (UGDATAUPDATEINTERVAL)
            else if (config.boardMode == 2) nextDataUpdate = millis() + 45000; // MODE_BUS (BUSDATAUPDATEINTERVAL)

            // Handle Result Codes
            if (lastUpdateResult == 0 || lastUpdateResult == 1) { // UPD_SUCCESS, UPD_NO_CHANGE
                displayMgr.setOtaUpdateAvailable(false);
                lastDataLoadTime = millis();
                noDataLoaded = false;
                dataLoadSuccess++;
                
                // Asynchronously update weather and RSS
                weatherClient& weather = context->getWeather();
                rssClient& rss = context->getRss();
                
                if (weather.getWeatherEnabled() && config.boardMode != 1 && millis() > weather.getNextWeatherUpdate()) {
                    float lat = (config.boardMode == 0) ? config.stationLat : config.busLat;
                    float lon = (config.boardMode == 0) ? config.stationLon : config.busLon;
                    this->updateCurrentWeather(lat, lon);
                }
                if (rss.getRssEnabled() && config.boardMode != 2 && millis() > rss.getNextRssUpdate()) {
                    updateRssFeed();
                }
                
                displayMgr.render();
            } else if (lastUpdateResult == 5) { // UPD_UNAUTHORISED
                displayMgr.showBoard(displayMgr.getSystemBoard(SystemBoardId::SYS_ERROR_TOKEN));
            } else {
                lastLoadFailure = millis();
                dataLoadFailure++;
                nextDataUpdate = millis() + 30000;
                displayMgr.setOtaUpdateAvailable(false);
                if (noDataLoaded) displayMgr.showBoard(displayMgr.getSystemBoard(SystemBoardId::SYS_ERROR_NO_DATA));
            }
        } else if (millis() > nextDataUpdate) {
            // Log once per minute why we aren't updating if WiFi is down
            static unsigned long lastWifiWaitLog = 0;
            if (millis() - lastWifiWaitLog > 60000) {
                LOG_INFO("DATA", "Waiting for WiFi to trigger data update. Connected: " + String(wifiConnected));
                lastWifiWaitLog = millis();
            }
        }

        if (firstLoad) {
            firstLoad = false;
            displayMgr.render();
        }
    }
}

/**
 * @brief Get the Build Timestamp of the running firmware
 * @return String formatted build timestamp.
 */
String systemManager::getBuildTime() {
  char timestamp[22];
  char buildtime[11];
  struct tm tm = {};

  sprintf(timestamp,"%s %s",__DATE__,__TIME__);
  strptime(timestamp,"%b %d %Y %H:%M:%S",&tm);
  sprintf(buildtime,"%02d%02d%02d%02d%02d",tm.tm_year-100,tm.tm_mon+1,tm.tm_mday,tm.tm_hour,tm.tm_min);
  return String(buildtime);
}

/**
 * @brief Trigger a manual update of the RSS feed headlines.
 */
void systemManager::updateRssFeed() {
    rssClient& rss = context->getRss();
    LOG_INFO("DATA", "Triggering RSS Feed update: " + String(rss.getRssURL()));
    int res = rss.loadFeed(rss.getRssURL());
    rss.setLastRssUpdateResult(res);
    LOG_INFO("DATA", "RSS Update Result: " + String(res == UPD_SUCCESS ? "SUCCESS" : "ERROR (" + String(res) + ")"));
    if (res == UPD_SUCCESS) rss.setNextRssUpdate(millis() + 600000); 
    else rss.setNextRssUpdate(millis() + 300000);
}

/**
 * @brief Performs a soft reload of the application state based on new configuration.
 */
void systemManager::softResetBoard() {
    ConfigManager& configMgr = context->getConfigManager();
    DisplayManager& displayMgr = context->getDisplayManager();
    rssClient& rss = context->getRss();
    weatherClient& weather = context->getWeather();
    const Config& config = configMgr.getConfig();
    
    String prevRssUrl = String(rss.getRssURL());
 
    configMgr.loadConfig();
    displayMgr.applyConfig(config);
    
    // Timezone sync
    if (timeManager.getTimezone() != "") {
        setenv("TZ", timeManager.getTimezone().c_str(), 1);
    } else {
        setenv("TZ", TimeManager::ukTimezone, 1);
    }
    tzset();
 
    // Reset states
    SplashBoard* splash = (SplashBoard*)displayMgr.getSystemBoard(SystemBoardId::SYS_BOOT_SPLASH);
    displayMgr.showBoard(splash);
 
    nextDataUpdate = 0;
    weather.setNextWeatherUpdate(0);
    displayMgr.resetState();
    firstLoad = true;
    noDataLoaded = true;
    prevProgressBarPosition = 70;
    startupProgressPercent = 70;
 
    rss.setRssAddedtoMsgs(false);
    if (rss.getRssEnabled() && prevRssUrl != rss.getRssURL()) {
        rss.numRssTitles = 0;
        if (config.boardMode == MODE_RAIL || config.boardMode == MODE_TUBE) {
            prevProgressBarPosition = 50;
            LoadingBoard* load = (LoadingBoard*)displayMgr.getSystemBoard(SystemBoardId::SYS_BOOT_LOADING);
            load->setProgress("Updating RSS headlines feed", 50);
            displayMgr.showBoard(load);
            updateRssFeed();
        }
    }
 
    if (config.boardMode == MODE_RAIL) {
        NationalRailBoard* nrb = (NationalRailBoard*)displayMgr.getDisplayBoard(0);
        if (nrb) nrb->setAltStationActive(setAlternateStation());
    }
    context->getGlobalMessagePool().clear();
}

/**
 * @brief Update the internal URL string for the Web GUI.
 */
void systemManager::updateMyUrl() {
    IPAddress ip = WiFi.localIP();
    snprintf(myUrl, sizeof(myUrl), "http://%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
}

/**
 * @brief Returns true if an alternate station is enabled and current time is within activation period.
 */
bool systemManager::isAltActive() {
    const Config& config = context->getConfigManager().getConfig();
    if (!config.altStationEnabled) return false;
    
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return false;

    byte myHour = timeinfo.tm_hour;
    if (config.altStarts > config.altEnds) {
        return (myHour >= config.altStarts) || (myHour < config.altEnds);
    } else {
        return (myHour >= config.altStarts) && (myHour < config.altEnds);
    }
}

/**
 * @brief Check schedule and set alternate station variables if in time range.
 */
bool systemManager::setAlternateStation() {
    const Config& config = context->getConfigManager().getConfig();
    if (config.boardMode == MODE_RAIL && config.altStationEnabled && isAltActive()) {
        NationalRailBoard* nrb = (NationalRailBoard*)context->getDisplayManager().getDisplayBoard(0);
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
 * @brief Callback from data clients to update UI progress during boot.
 */
void systemManager::tflCallback() {
    const Config& config = context->getConfigManager().getConfig();
    if (firstLoad) {
        if (startupProgressPercent < 95) {
            startupProgressPercent += 5;
            LoadingBoard* load = (LoadingBoard*)context->getDisplayManager().getSystemBoard(SystemBoardId::SYS_BOOT_LOADING);
            if (load) {
                if (config.boardMode == MODE_TUBE) load->setProgress("Initialising TfL interface", startupProgressPercent);
                else load->setProgress("Initialising BusTimes interface", startupProgressPercent);
                context->getDisplayManager().showBoard(load);
                context->getDisplayManager().resetState();
            }
        }
    } else {
        context->getDisplayManager().yieldAnimationUpdate();
    }
}

/**
 * @brief Callback from National Rail data client to keep UI responsive.
 */
void systemManager::raildataCallback(int stage, int nServices) {
    if (firstLoad) {
        int percent = ((nServices * 20) / 40) + 80;
        LoadingBoard* load = (LoadingBoard*)context->getDisplayManager().getSystemBoard(SystemBoardId::SYS_BOOT_LOADING);
        load->setProgress("Initialising National Rail interface", percent);
        context->getDisplayManager().showBoard(load);
    } else {
        context->getDisplayManager().yieldAnimationUpdate();
    }
}

/**
 * @brief Proxy to correctly format and trigger a weather update via the weatherClient.
 */
void systemManager::updateCurrentWeather(float lat, float lon) {
    weatherClient& weather = context->getWeather();
    if (weather.getWeatherEnabled()) {
        char latStr[16], lonStr[16];
        dtostrf(lat, 4, 4, latStr);
        dtostrf(lon, 4, 4, lonStr);
        weather.updateWeather(weather.getOpenWeatherMapApiKey(), latStr, lonStr);
    }
}

/**
 * @brief Append RSS headlines to the scrolling message pool.
 */
void systemManager::addRssMessage() {
    rssClient& rss = context->getRss();
    MessagePool& pool = context->getGlobalMessagePool();
    const Config& config = context->getConfigManager().getConfig();
    
    if (rss.getRssEnabled() && pool.getCount() < 4 && rss.numRssTitles > 0) {
        String rssCombined = String(rss.getRssName()) + ": " + rss.rssTitle[0];
        
        for (int i = 1; i < rss.numRssTitles; i++) {
            if (rssCombined.length() + strlen(rss.rssTitle[i]) + 2 < 400) {
                rssCombined += (config.boardMode == MODE_TUBE) ? "\x81" : "\x90";
                rssCombined += rss.rssTitle[i];
            } else {
                break;
            }
        }
        pool.addMessage(rssCombined.c_str());
        rss.setRssAddedtoMsgs(true);
    }
}

/**
 * @brief Clean up RSS messages from the board message pool.
 */
void systemManager::removeRssMessage() {
    rssClient& rss = context->getRss();
    if (rss.getRssAddedtoMsgs()) {
        context->getGlobalMessagePool().removeLastMessage();
        rss.setRssAddedtoMsgs(false);
    }
}
