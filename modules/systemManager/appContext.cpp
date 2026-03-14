/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/systemManager/appContext.cpp
 * Description: Implementation of the central appContext orchestrator.
 *
 * Exported Functions/Classes:
 * - appContext::appContext: Constructor initializes core state.
 * - appContext::begin: Master initialization sequence for all hardware and software services.
 * - appContext::tick: Central administrative loop for high-level tasks.
 * - yieldCallbackWrapper: Global yield relay for non-blocking I/O.
 * - raildataYieldWrapper: Global progress callback relay for National Rail.
 */

#include "appContext.hpp"
#include "systemManager.hpp"
#include <timeManager.hpp>
#include <Logger.hpp>
#include <WiFiConfig.hpp>

appContext* _instance = nullptr;

/**
 * @brief Global wrapper to trigger DisplayManager's non-blocking yield.
 *        Required for raw function pointers used in data clients.
 */
void yieldCallbackWrapper() {
    if (_instance) _instance->getDisplayManager().yieldAnimationUpdate();
}

/**
 * @brief Adaptor for National Rail data source which provides progress events.
 *        Relays to the central yield handler.
 */
void raildataYieldWrapper(int stage, int nServices) {
    yieldCallbackWrapper();
}

/**
 * @brief Construct the appContext and its managed service singletons.
 */
appContext::appContext() : globalMessagePool(4) {
    // Note: Managers are initialized via their default constructors here.
}

/**
 * @brief Initialize all system services in the required boot order.
 */
void appContext::begin() {
    _instance = this;
    LOG_INFO("SYSTEM", "Initializing appContext managers...");

    // 1. Hardware Filesystem (Must be first for config)
    LOG_INFO("SYSTEM", "Initializing LittleFS...");
    LittleFS.begin(true);

    // 2. Storage & Config
    LOG_INFO("SYSTEM", "Loading API keys and configuration...");
    configManager.loadApiKeys();
    configManager.loadConfig();
    const Config& config = configManager.getConfig();

    // 3. Networking (WiFi)
    LOG_INFO("SYSTEM", "Initializing WiFi...");
    wifiManager.begin(config.hostname);

    // 4. Hardware Display
    LOG_INFO("SYSTEM", "Initializing DisplayManager...");
    displayManager.begin(this);

    // 5. System Management (State & Refresh)
    LOG_INFO("SYSTEM", "Initializing systemManager...");
    sysManager.begin(this);

    LOG_INFO("SYSTEM", "Network state: SSID=" + WiFi.SSID() + ", IP=" + WiFi.localIP().toString());
    
    struct tm timeinfo;
    if(getLocalTime(&timeinfo)) {
        char timeStr[32];
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
        LOG_INFO("SYSTEM", String("Current time: ") + timeStr);
    }

    // 6. Register Config Consumers
    LOG_INFO("SYSTEM", "Registering Configuration Consumers...");
    configManager.registerConsumer(&displayManager);
    configManager.registerConsumer(&weather);
    configManager.registerConsumer(&rss);
    configManager.registerConsumer(&otaAssetUpdater);
    configManager.registerConsumer(&timeManager);
    configManager.registerConsumer(&wifiManager);

    // 6. Initialize global helper modules
    LOG_INFO("SYSTEM", "Initializing Hardware Display...");
    otaAssetUpdater.init(this);
    timeManager.init(this);

    LOG_INFO("SYSTEM", "Stabilizing network stack...");
    delay(2000);

    LOG_INFO("SYSTEM", "Initializing Web Server...");
    webServer.init();

    // 7. Connect Yield Callbacks for Non-Blocking I/O
    weather.setYieldCallback(yieldCallbackWrapper);
    rss.setYieldCallback(yieldCallbackWrapper);

    LOG_INFO("SYSTEM", "Network ready. IP Address: " + WiFi.localIP().toString());

    LOG_INFO("SYSTEM", "Notifying consumers to apply initial configuration...");
    configManager.notifyConsumersToReapplyConfig();

    LOG_INFO("SYSTEM", "appContext initialization complete.");
}

/**
 * @brief Dispatch periodic maintenance ticks to all active services.
 */
void appContext::tick() {
    // 1. Firmware update lifecycle
    otaAssetUpdater.tick();

    // 2. System state and refresh logic
    sysManager.tick();

    // 3. Display rendering tick
    displayManager.tick(millis());

    // 4. DNS hijacked processing (Captive Portal)
    wifiManager.processDNS();

    // 5. Web server handle
    webServer.handleClient();
}
