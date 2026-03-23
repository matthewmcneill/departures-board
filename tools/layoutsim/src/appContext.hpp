/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: tools/layoutsim/src/appContext.hpp
 * Description: Mock application context for the layout simulator, providing access to mocked time and configuration managers.
 *
 * Exported Functions/Classes:
 * - Config: Mock configuration data structure
 * - ConfigManager: Mock manager for configuration data
 *   - getConfig(): Retrieves the mock configuration instance
 * - appContext: Main mock context container
 *   - getTimeManager(): Retrieves the mock time manager instance
 *   - getConfigManager(): Retrieves the mock config manager instance
 * - weekdays: Array of short weekday names
 */

#ifndef APP_CONTEXT_MOCK_HPP
#define APP_CONTEXT_MOCK_HPP

#include "timeManager.hpp"
#include <string>

class Config {
public:
    bool dateEnabled = true; // Determines if the date is displayed on applicable widgets
};

class ConfigManager {
public:
    /**
     * @brief Retrieves the mock configuration instance
     * @return Reference to the static Config object
     */
    Config& getConfig() { static Config c; return c; }
};

class appContext {
public:
    /**
     * @brief Retrieves the mock time manager instance
     * @return Reference to the global TimeManager object
     */
    TimeManager& getTimeManager() { 
        extern TimeManager* g_timeMgr;
        return *g_timeMgr; 
    }
    
    /**
     * @brief Retrieves the mock config manager instance
     * @return Reference to the static ConfigManager object
     */
    ConfigManager& getConfigManager() { static ConfigManager cm; return cm; }
};

extern class appContext appContext; // Global context instance
extern const char* weekdays[]; // Array of short weekday names

#endif
