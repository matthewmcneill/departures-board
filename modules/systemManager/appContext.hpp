/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * DESIGN RATIONALE:
 * This class serves as the central "System Hub" for several critical technical reasons:
 * 1. Dependency Injection: It allows modules to receive a pointer to the context 
 *    rather than being "hard-pinned" to global variables in the main file.
 * 2. Deterministic Boot: It ensures a predictable initialization sequence across 
 *    different translation units, avoiding the C++ "Static Initialization Order Fiasco".
 * 3. Service Discovery: It provides a clean, header-based entry point for modules 
 *    to find and interact with other system services (Config, Display, Network).
 * 
 * For a deep dive into this architecture, see doc/ArchitectureAndEncapsulation.md.
 *
 * Module: modules/systemManager/appContext.hpp
 * Description: Central context orchestrator that owns core managers and services.
 *              Serves as the primary dependency injection point for the system.
 *
 * Exported Functions/Classes:
 * - appContext: Lifecycle owner for Config, Display, Network, and OTA managers.
 * - yieldCallbackWrapper: Static wrapper to trigger display yield from any context.
 * - raildataYieldWrapper: Callback adaptor for National Rail data source events.
 */

#pragma once

#include <configManager.hpp>
#include <displayManager.hpp>
#include <otaUpdater.hpp>
#include <webServer.hpp>
#include <weatherClient.hpp>
#include <rssClient.hpp>
#include "systemManager.hpp"
#include <messaging/messagePool.hpp>

/**
 * @brief High-level states for the AppContext Hierarchical State Machine.
 */
enum class AppState {
    BOOTING,         // Initializing FS, Configs, Boot Splash
    WIFI_SETUP,      // WiFiManager is in AP mode. Show captive portal instructions.
    BOARD_SETUP,     // WiFi connected, but config.boards is empty. Show "Please configure" screen.
    RUNNING          // Normal operation.
};

/**
 * @brief Orchestrator for all core system services and shared state.
 */
class appContext {
private:
    ConfigManager configManager;    ///< Persistence and settings management
    DisplayManager displayManager;  ///< Rendering and board lifecycle
    otaUpdater otaAssetUpdater;     ///< Firmware update lifecycle
    WebServerManager webServer;     ///< Local GUI and API service
    systemManager sysManager;       ///< Global application and network state
    weatherClient weather;          ///< External weather conditions client
    rssClient rss;                  ///< News feed scroller client
    MessagePool globalMessagePool;  ///< Shared pool for scrolling status messages
    AppState currentState;          ///< Current tracked system state
    bool webServerInitialized;      ///< Checks if webServer was safely started

public:
    /**
     * @brief Construct the application context.
     */
    appContext();

    /**
     * @brief Initialize all sub-managers in the correct dependency order.
     */
    void begin();

    /**
     * @brief Main administrative tick to dispatch lifecycle events to all managers.
     */
    void tick();

    // --- Service Accessors (DI Points) ---
    ConfigManager& getConfigManager() { return configManager; }
    DisplayManager& getDisplayManager() { return displayManager; }
    otaUpdater& getOtaUpdater() { return otaAssetUpdater; }
    WebServerManager& getWebServer() { return webServer; }
    systemManager& getsystemManager() { return sysManager; }
    weatherClient& getWeather() { return weather; }
    rssClient& getRss() { return rss; }
    MessagePool& getGlobalMessagePool() { return globalMessagePool; }
    AppState getAppState() const { return currentState; }
};

// Global yield wrappers for non-blocking I/O

/**
 * @brief Global wrapper to trigger DisplayManager's non-blocking yield.
 *        Used by data clients to keep the web server alive during long I/O.
 */
void yieldCallbackWrapper();

/**
 * @brief Adaptor for National Rail data source which provides progress events.
 * @param stage Current parsing stage.
 * @param nServices Number of services found so far.
 */
void raildataYieldWrapper(int stage, int nServices);
