/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
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
 * Module: modules/appContext/appContext.hpp
 * Description: Central context orchestrator that owns core managers and services.
 *              Serves as the primary dependency injection point for the system.
 *
 * Exported Functions/Classes:
 * - appContext: [Class] Lifecycle owner for Config, Display, Network, and OTA managers.
 *   - begin(): Master initialization sequence for all hardware and FS services.
 *   - tick(): Central administrative loop for state transitions and maintenance.
 *   - softResetBoard(): Signals all managers to perform a non-volatile reload.
 *   - getBuildTime(): Returns the firmware version string (YYMMDDHHMM).
 *   - setInputDevice(std::unique_ptr<buttonHandler>): Attaches an interaction driver.
 *   - getConfigManager()/getDisplayManager()/getWebServer(): Service discovery points.
 *   - getAppState(): Returns the current system operating mode (BOOTING, RUNNING, etc).
 */

#pragma once

#include <memory>

#include <configManager.hpp>
#include <displayManager.hpp>
#include <otaUpdateManager.hpp>
#include <webServer.hpp>
#include <weatherClient.hpp>
#include <rssClient.hpp>
#include <wifiManager.hpp>
#include <timeManager.hpp>
#include <messaging/messagePool.hpp>
#include <dataManager.hpp>
#include "schedulerManager.hpp"
#include <deviceCrypto.hpp>

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
    DeviceCrypto deviceCrypto;      ///< Persistent Master Key Cryptographic Engine
    ConfigManager configManager;    ///< Persistence and settings management
    dataManager networkManager;     ///< Centralized queue for background HTTP fetches
    DisplayManager displayManager;  ///< Rendering and board lifecycle
    otaUpdateManager otaAssetUpdater;     ///< Firmware update lifecycle
    WebServerManager webServer;     ///< Local GUI and API service
    weatherClient weather;          ///< External weather conditions client
    rssClient rss;                  ///< News feed scroller client
    WifiManager wifiManager;        ///< WiFi configuration and connectivity manager
    TimeManager timeManager;        ///< System clock and NTP manager
    MessagePool globalMessagePool;  ///< Shared pool for scrolling status messages
    schedulerManager schedule;      ///< Evaluates which display boards to show
    AppState currentState;          ///< Current tracked system state
    bool webServerInitialized;      ///< Checks if webServer was safely started
    
    // --- Migrated from systemManager ---
    bool firstLoad;                 ///< True during initial boot sequence
    int startupProgressPercent;     ///< Aggregated boot progress (0-100)
    int prevProgressBarPosition;    ///< Cached UI value to avoid redraw flicker
    std::unique_ptr<class buttonHandler> inputDevice; ///< Generic input device for interaction

public:
    /**
     * @brief Construct the application context.
     */
    appContext();

    /**
     * @brief Essential for RAII unique_ptr members with forward declarations.
     */
    ~appContext();

    /**
     * @brief Initialize all sub-managers in the correct dependency order.
     */
    void begin();

    /**
     * @brief Main administrative tick to dispatch lifecycle events to all managers.
     */
    void tick();

    /**
     * @brief Helper to update the visual booting progress bar.
     */
    void updateBootProgress(int percentage, const char *msg);

    // --- Service Accessors (DI Points) ---
    /** @brief Get the cryptographic engine. */
    DeviceCrypto& getDeviceCrypto() { return deviceCrypto; }
    /** @brief Get the configuration persistence manager. */
    ConfigManager& getConfigManager() { return configManager; }
    /** @brief Get the display and rendering manager. */
    DisplayManager& getDisplayManager() { return displayManager; }
    /** @brief Get the firmware OTA updater. */
    otaUpdateManager& getOtaUpdater() { return otaAssetUpdater; }
    /** @brief Get the local web server and API manager. */
    WebServerManager& getWebServer() { return webServer; }
    /** @brief Get the weather external API client. */
    weatherClient& getWeather() { return weather; }
    /** @brief Get the WiFi connection manager. */
    WifiManager& getWifiManager() { return wifiManager; }
    /** @brief Get the system clock and NTP manager. */
    TimeManager& getTimeManager() { return timeManager; }
    /** @brief Get the news feed external API client. */
    rssClient& getRss() { return rss; }
    /** @brief Get the centralized background data manager. */
    dataManager& getDataManager() { return networkManager; }
    /** @brief Get the centralized scheduler manager. */
    schedulerManager& getSchedulerManager() { return schedule; }
    /** @brief Get the shared message pool for scrolling text. */
    MessagePool& getGlobalMessagePool() { return globalMessagePool; }
    /** @brief Get the current high-level state of the application. */
    AppState getAppState() const { return currentState; }

    /**
     * @brief Performs a soft reload of the application state based on new
     * configuration. Signals all consumers to refresh themselves.
     */
    void softResetBoard();

    /**
     * @brief Get the Build Timestamp of the running firmware.
     * @return String formatted build timestamp (YYMMDDHHMM).
     */
    String getBuildTime();

    /** @brief Set the hardware input device for interaction. */
    void setInputDevice(std::unique_ptr<class buttonHandler> device);

    /** @brief Get the boot progress percentage. */
    int getStartupProgressPercent() const { return startupProgressPercent; }
    /** @brief Set the boot progress percentage. */
    void setStartupProgressPercent(int percent) { startupProgressPercent = percent; }
    
    /** @brief Check if this is the first system load. */
    bool getFirstLoad() const { return firstLoad; }
    /** @brief Set the first load flag. */
    void setFirstLoad(bool load) { firstLoad = load; }
};

// Global yield wrappers for non-blocking I/O - DEPRECATED for v3.0 FreeRTOS flow
