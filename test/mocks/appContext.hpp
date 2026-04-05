#ifndef APP_CONTEXT_MOCK_HPP
#define APP_CONTEXT_MOCK_HPP

#include "Arduino.h"
#include "WiFi.h"
#include <vector>
#include <string>
#include "SystemState.hpp"

// Forward declarations
class ConfigManager;
class TimeManager;
class schedulerManager;
class dataManager;
class displayManager;
class wifiManager;
class weatherClient;
class rssClient;
class MessagePool;
class Portal;

class appContext {
public:
    appContext();
    ~appContext();

    ConfigManager& getConfigManager();
    TimeManager& getTimeManager();
    schedulerManager& getSchedulerManager();
    dataManager& getDataManager();
    displayManager& getDisplayManager();
    wifiManager& getWifiManager();
    weatherClient& getWeather();
    rssClient& getRss();
    MessagePool& getGlobalMessagePool();
    Portal& getPortal();

    // Mock lifecycle overrides
    void begin();
    void tick();
};

extern appContext g_appContext;

#endif
