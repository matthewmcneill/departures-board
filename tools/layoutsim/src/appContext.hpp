#ifndef APP_CONTEXT_MOCK_HPP
#define APP_CONTEXT_MOCK_HPP

#include "timeManager.hpp"
#include <string>

class Config {
public:
    bool dateEnabled = true;
};

class ConfigManager {
public:
    Config& getConfig() { static Config c; return c; }
};

class appContext {
public:
    TimeManager& getTimeManager() { 
        extern TimeManager* g_timeMgr;
        return *g_timeMgr; 
    }
    ConfigManager& getConfigManager() { static ConfigManager cm; return cm; }
};

extern class appContext appContext;
extern const char* weekdays[];

#endif
