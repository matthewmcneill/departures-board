/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/systemManager/systemManager.cpp
 * Description: Implementation of the systemManager for global state and logic.
 *
 * Exported Functions/Classes:
 * - systemManager: Lifecycle owner for non-display system tasks.
 *   - begin(): Initialize with appContext.
 *   - tick(): Main administrative logic loop.
 *   - softResetBoard(): Performs a soft reconfiguration and reboot of the state.
 */

#include "systemManager.hpp"
#include "appContext.hpp"
#include <logger.hpp>
#include <timeManager.hpp>
#include <boards/nationalRailBoard/nationalRailBoard.hpp>
#include <wifiManager.hpp> // Added as per instruction
#include <buttonHandler.hpp>
#include "buildTime.hpp"
#include "schedulerManager.hpp"

/**
 * @brief Initialize default system state.
 */
systemManager::systemManager() 
    : context(nullptr), inputDevice(nullptr), wifiConfigured(false), wifiConnected(false), 
      lastWiFiReconnect(0), firstLoad(true), startupProgressPercent(0), 
      prevProgressBarPosition(0), nextRoundRobinUpdate(0), backgroundUpdateIndex(0), lastDataLoadTime(0), 
      noDataLoaded(true), dataLoadSuccess(0), dataLoadFailure(0), 
      lastLoadFailure(0), lastUpdateResult(0), lastActiveSlotIndex(-1) {
    strlcpy(myUrl, "http://0.0.0.0", sizeof(myUrl));
}

systemManager::~systemManager() {
    if (inputDevice) {
        delete inputDevice;
        inputDevice = nullptr;
    }
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
    // Process input
    if (inputDevice) {
        inputDevice->update();
        if (inputDevice->wasShortTapped()) {
            DisplayManager& displayMgr = context->getDisplayManager();
            if (displayMgr.getIsSleeping() || displayMgr.getForcedSleep()) {
                LOG_INFO("INPUT", "Short tap: Waking from sleep");
                displayMgr.setForcedSleep(false);
                displayMgr.resumeDisplays();
            } else {
                LOG_INFO("INPUT", "Short tap: Cycling mode");
                displayMgr.cycleNext();
            }
        } else if (inputDevice->wasLongTapped()) {
            DisplayManager& displayMgr = context->getDisplayManager();
            bool currentlySleeping = displayMgr.getForcedSleep() || displayMgr.getIsSleeping();
            LOG_INFO("INPUT", String("Long tap: Toggling screensaver. Currently sleeping: ") + currentlySleeping);
            displayMgr.setForcedSleep(!currentlySleeping);
            if (!displayMgr.getForcedSleep()) {
                displayMgr.resumeDisplays();
            }
        }
    }

    // Connection Management
    if (WiFi.status() != WL_CONNECTED && wifiConnected) {
        wifiConnected = false;
        wifiDisconnectTimer = millis(); // Start tracking downtime
        LOG_WARN("SYSTEM", "WiFi connection lost.");
    } else if (WiFi.status() == WL_CONNECTED && !wifiConnected) {
        wifiConnected = true;
        wifiDisconnectTimer = 0; // Reset downtime tracker
        LOG_INFO("SYSTEM", "WiFi connected. IP: " + WiFi.localIP().toString());
        updateMyUrl();
        // Immediately try to refresh data upon reconnection if we were waiting
        if (noDataLoaded ||  millis() > getNextRoundRobinUpdate()) {
            nextRoundRobinUpdate = millis();
        }
    }

    /* 
     * DEPRECATED: WiFi reconnection is now handled by the WifiManager state machine
     * to prevent conflicts with testConnection() and other async network tasks.
     * 
    if (WiFi.status() != WL_CONNECTED && millis() > lastWiFiReconnect + 10000) {
        if (!context->getWifiManager().getAPMode()) {
            LOG_INFO("SYSTEM", "Attempting WiFi reconnection...");
            WiFi.disconnect();
            delay(100);
            WiFi.reconnect();
            lastWiFiReconnect = millis();
        }
    }
    */

    DisplayManager& displayMgr = context->getDisplayManager();
    const Config& config = context->getConfigManager().getConfig();

    // Data Fetching Logic (Only fetch when not sleeping and initialized)
    if (!displayMgr.getIsSleeping() && config.boardCount > 0) {
        int activeIndex = displayMgr.getActiveSlotIndex();
        
        // Detect board switch to trigger immediate update (Fast-Path override)
        if (activeIndex != lastActiveSlotIndex) {
            LOG_INFO("SYSTEM", "Board switch detected. Aligning sweeper.");
            iDisplayBoard* activeBoard = displayMgr.getDisplayBoard(activeIndex);
            if (activeBoard && config.boards[activeIndex].complete) {
                int status = activeBoard->getLastUpdateStatus();
                // Only trigger fetch if unloaded, pending, or errored to prevent API flooding from rapid swiping
                if (status == -1 || status == 9 || status > 2) {
                    LOG_INFO("SYSTEM", "Active board unloaded or errored. Triggering background fetch.");
                    activeBoard->updateData();
                } else {
                    LOG_INFO("SYSTEM", "Active board has recent data. Skipping fetch.");
                }
            }
            
            // Align the background sweeper to prioritize monitoring the newly active board
            backgroundUpdateIndex = activeIndex;
            nextRoundRobinUpdate = millis() + 500;
            
            lastActiveSlotIndex = activeIndex;
        }

        // Rotational Background Data Fetching Logic
        if (millis() > nextRoundRobinUpdate && wifiConnected && context->getAppState() == AppState::RUNNING) {
            // Omitted OTA flag toggle 
            
            const BoardConfig& bgConfig = config.boards[backgroundUpdateIndex];
            iDisplayBoard* bgBoard = displayMgr.getDisplayBoard(backgroundUpdateIndex);
            
            if (bgBoard && bgConfig.complete) {
                lastUpdateResult = bgBoard->updateData(); // This both initiates pulls AND polls pending locks
                
                if (bgBoard->getLastUpdateStatus() == 9) { // 9 == UPD_PENDING
                    // Lock cursor on this pending board and poll it again in 500ms
                    nextRoundRobinUpdate = millis() + 500;
                } else {
                    int distributedInterval = config.apiRefreshRate / config.boardCount;
                    if (distributedInterval < 10000) distributedInterval = 10000; // Hard minimum floor
                    
                    if (bgBoard->getLastUpdateStatus() == -1) {
                        // Fast-fill initialization override for unloaded dashboards
                        LOG_INFO("DATA", "Fast-Filling unloaded board index array: " + String(backgroundUpdateIndex));
                        distributedInterval = 2000; 
                    } else {
                        LOG_INFO("DATA", "Background sweep dispatched API request for index: " + String(backgroundUpdateIndex));
                    }
                    
                    nextRoundRobinUpdate = millis() + distributedInterval;
                    
                    // Proceed to next carousels only after completion
                    if (config.boardCount > 0) {
                        backgroundUpdateIndex = (backgroundUpdateIndex + 1) % config.boardCount;
                    }
                }
            } else {
                // Skip empty slots manually
                nextRoundRobinUpdate = millis() + 500;
                if (config.boardCount > 0) {
                    backgroundUpdateIndex = (backgroundUpdateIndex + 1) % config.boardCount;
                }
            } // Handle Result Codes
                if (lastUpdateResult == 0 || lastUpdateResult == 1) { // UPD_SUCCESS, UPD_NO_CHANGE
                    // Omitted OTA flag toggle
                    lastDataLoadTime = millis();
                    noDataLoaded = false;
                    dataLoadSuccess++;
                    
                    // Asynchronously update weather and RSS
                    weatherClient& weather = context->getWeather();
                    rssClient& rss = context->getRss();
                    
                    const BoardConfig& activeBC = config.boards[activeIndex];
                    
                    if (weather.getWeatherEnabled() && activeBC.showWeather && millis() > weather.getNextWeatherUpdate()) {
                        iDisplayBoard* active = displayMgr.getDisplayBoard(activeIndex);
                        if (active) {
                            WeatherStatus& ws = active->getWeatherStatus();
                            if (activeBC.lat == 0.0f && activeBC.lon == 0.0f) {
                                LOG_WARN("DATA", "Weather enabled but coordinates are 0,0. Skipping.");
                                ws.status = WeatherUpdateStatus::NOT_CONFIGURED;
                            } else {
                                ws.lat = activeBC.lat;
                                ws.lon = activeBC.lon;
                                weather.updateWeather(ws, activeBC.apiKeyId);
                            }
                        }
                    }
                    if (rss.getRssEnabled() && activeBC.type != MODE_BUS && millis() > rss.getNextRssUpdate()) {
                        updateRssFeed();
                    }
                    
                    displayMgr.render();
                } else if (lastUpdateResult == 5) { // UPD_UNAUTHORISED
                    // Board must render its own error inline; do not hijack the display here.
                    LOG_WARN("DATA", "API Unauthorised. Displaying inline warning.");
                } else {
                    lastLoadFailure = millis();
                    dataLoadFailure++;
                }
        } else if (millis() > nextRoundRobinUpdate && context->getAppState() == AppState::RUNNING) {
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
#ifdef BUILD_TIME
  return String(BUILD_TIME);
#else
  char timestamp[22];
  char buildtime[11];
  struct tm tm = {};

  sprintf(timestamp,"%s %s",__DATE__,__TIME__);
  strptime(timestamp,"%b %d %Y %H:%M:%S",&tm);
  sprintf(buildtime,"%02d%02d%02d%02d%02d",tm.tm_year-100,tm.tm_mon+1,tm.tm_mday,tm.tm_hour,tm.tm_min);
  return String(buildtime);
#endif
}

/**
 * @brief Trigger a manual update of the RSS feed headlines.
 */
void systemManager::updateRssFeed() {
    rssClient& rss = context->getRss();
    if (rss.getLastRssUpdateResult() == 9) return; // Wait until current fetch completes
    LOG_INFO("DATA", "Triggering RSS Feed update: " + String(rss.getRssURL()));
    rss.loadFeed(rss.getRssURL());
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
    if (context->getTimeManager().getTimezone() != "") {
        setenv("TZ", context->getTimeManager().getTimezone().c_str(), 1);
    } else {
        setenv("TZ", TimeManager::ukTimezone, 1);
    }
    tzset();
 
    // Reset states
    if (onSoftReset) {
        onSoftReset();
    }
 
    nextRoundRobinUpdate = 0;
    backgroundUpdateIndex = 0;
    weather.setNextWeatherUpdate(0);
    displayMgr.resumeDisplays();
    firstLoad = true;
    noDataLoaded = true;
    prevProgressBarPosition = 70;
    startupProgressPercent = 70;
 
    rss.setRssAddedtoMsgs(false);
    if (rss.getRssEnabled() && prevRssUrl != rss.getRssURL()) {
        rss.numRssTitles = 0;
        if (config.boardCount > 0 && (config.boards[0].type == MODE_RAIL || config.boards[0].type == MODE_TUBE)) {
            prevProgressBarPosition = 50;
            if (onBootProgress) {
                onBootProgress("Updating RSS headlines feed", 50);
            }
            updateRssFeed();
        }
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
 * @brief Returns true if WiFi has been disconnected for longer than the timeout period (3 minutes).
 */
bool systemManager::isWifiPersistentError() const {
    if (wifiConnected) return false;
    // Over 3 minutes of continuous disconnection
    return (millis() - wifiDisconnectTimer > 180000);
}


/**
 * @brief Callback from data clients to update UI progress during boot.
 */
void systemManager::tflCallback() {
    const Config& config = context->getConfigManager().getConfig();
    if (firstLoad) {
        if (startupProgressPercent < 95) {
            startupProgressPercent += 5;
            if (onBootProgress) {
                BoardTypes firstType = (config.boardCount > 0) ? config.boards[0].type : MODE_RAIL;
                if (firstType == MODE_TUBE) onBootProgress("Initialising TfL interface", startupProgressPercent);
                else onBootProgress("Initialising BusTimes interface", startupProgressPercent);
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
        if (onBootProgress) {
            onBootProgress("Initialising National Rail interface", percent);
        }
    } else {
        context->getDisplayManager().yieldAnimationUpdate();
    }
}

/**
 * @brief Proxy to correctly format and trigger a weather update via the weatherClient.
 */
/**
 * @brief Proxy to correctly format and trigger a weather update via the weatherClient.
 *        Note: This legacy proxy is now partially deprecated by the direct board injection above, 
 *              but kept for manual triggers if needed.
 */
void systemManager::updateCurrentWeather(float lat, float lon) {
    weatherClient& weather = context->getWeather();
    if (weather.getWeatherEnabled()) {
        int activeIndex = context->getDisplayManager().getActiveSlotIndex();
        iDisplayBoard* active = context->getDisplayManager().getDisplayBoard(activeIndex);
        if (active) {
            WeatherStatus& ws = active->getWeatherStatus();
            ws.lat = lat;
            ws.lon = lon;
            const BoardConfig& bc = context->getConfigManager().getConfig().boards[activeIndex];
            weather.updateWeather(ws, bc.apiKeyId);
        }
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
                BoardTypes firstType = (config.boardCount > 0) ? config.boards[0].type : MODE_RAIL;
                rssCombined += (firstType == MODE_TUBE) ? "\x81" : "\x90";
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
