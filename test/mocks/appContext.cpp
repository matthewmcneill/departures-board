#include "appContext.hpp"

// Mock Manager Includes
#include "configManager.hpp"
#include "timeManager.hpp"
#include "schedulerManager.hpp"
#include "dataManager.hpp"
#include "displayManager.hpp"
#include "wifiManager.hpp"
#include "weatherClient.hpp"
#include "rssClient.hpp"
#include "MessagePool.hpp"
#include "Portal.hpp"

appContext::appContext() {}
appContext::~appContext() {}

ConfigManager& appContext::getConfigManager() { static ConfigManager cm; return cm; }
TimeManager& appContext::getTimeManager() { static TimeManager tm; return tm; }
schedulerManager& appContext::getSchedulerManager() { static schedulerManager sm(this); return sm; }
dataManager& appContext::getDataManager() { static dataManager dm; return dm; }
displayManager& appContext::getDisplayManager() { static displayManager dsm(*this); return dsm; }
wifiManager& appContext::getWifiManager() { static wifiManager wm; return wm; }
weatherClient& appContext::getWeather() { static weatherClient wc; return wc; }
rssClient& appContext::getRss() { static rssClient rc; return rc; }
MessagePool& appContext::getGlobalMessagePool() { static MessagePool mp; return mp; }
Portal& appContext::getPortal() { static Portal p; return p; }

void appContext::begin() {}
void appContext::tick() {}

appContext g_appContext;
