/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/appContext/appContext.cpp
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
#include <logger.hpp>
#include <wifiManager.hpp>
#include <boards/systemBoard/loadingBoard.hpp>
#include <boards/systemBoard/splashBoard.hpp>
#include <boards/systemBoard/firmwareUpdateBoard.hpp>

appContext* _instance = nullptr; // Global static pointer for yield callback adaptor

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
appContext::appContext() : globalMessagePool(4), currentState(AppState::BOOTING), webServerInitialized(false) {
    // Note: Managers are initialized via their default constructors here.
}

/**
 * @brief Initialize all system services in the required boot order.
 */
void appContext::begin() {
    _instance = this;
    LOG_SPLASH("APP STATE: BOOTING");
    LOG_INFO("SYSTEM", "Initializing appContext managers...");

    // 1. Hardware Display (Must be first for boot visuals/stability)
    LOG_INFO("SYSTEM", "Initializing DisplayManager...");
    displayManager.begin(this);

    auto updateBootProgress = [&](int percentage) {
        if (auto* splash = static_cast<SplashBoard*>(displayManager.getSystemBoard(SystemBoardId::SYS_BOOT_SPLASH))) {
            splash->setProgress(percentage);
            displayManager.render();
        }
    };
    
    updateBootProgress(10);

    // 2. Hardware Filesystem (Must be first for config)
    LOG_INFO("SYSTEM", "Initializing LittleFS...");
    LittleFS.begin(true);
    updateBootProgress(30);

    // 3. Storage & Config
    LOG_INFO("SYSTEM", "Loading API keys and configuration...");
    configManager.loadApiKeys();
    configManager.loadConfig();
    const Config& config = configManager.getConfig();
    updateBootProgress(50);

    // 4. Networking (WiFi)
    LOG_INFO("SYSTEM", "Initializing WiFi...");
    wifiManager.begin(config.hostname);
    updateBootProgress(70);

    // 5. System Management (State & Refresh)
    LOG_INFO("SYSTEM", "Initializing systemManager...");
    sysManager.begin(this);
    updateBootProgress(85);

    LOG_INFO("SYSTEM", "Network state: SSID=" + String(WiFi.SSID()) + ", IP=" + WiFi.localIP().toString());

    // 6. Register Config Consumers
    LOG_INFO("SYSTEM", "Registering Configuration Consumers...");
    configManager.registerConsumer(&displayManager);
    configManager.registerConsumer(&weather);
    configManager.registerConsumer(&rss);
    configManager.registerConsumer(&otaAssetUpdater);
    configManager.registerConsumer(&timeManager);
    configManager.registerConsumer(&wifiManager);
    updateBootProgress(95);

    // 7. Initialize global helper modules
    LOG_INFO("SYSTEM", "Initializing helper modules...");
    otaAssetUpdater.init(this);
    timeManager.init(this);
    updateBootProgress(100);

    LOG_INFO("SYSTEM", "Stabilizing system...");
    // 8. Connect Yield Callbacks for Non-Blocking I/O
    weather.setYieldCallback(yieldCallbackWrapper);
    rss.setYieldCallback(yieldCallbackWrapper);

    // 9. Connect Observer Callbacks for UI Decoupling
    sysManager.setBootProgressCallback([this](const char* msg, int percent) {
        if (auto* load = static_cast<LoadingBoard*>(displayManager.getSystemBoard(SystemBoardId::SYS_BOOT_LOADING))) {
            load->setProgress(msg, percent);
            displayManager.showBoard(load, "systemManager Boot Progress Hook");
            displayManager.resetState();
        }
    });

    sysManager.setSoftResetCallback([this]() {
        currentState = AppState::BOOTING;
        if (auto* splash = static_cast<SplashBoard*>(displayManager.getSystemBoard(SystemBoardId::SYS_BOOT_SPLASH))) {
            displayManager.showBoard(splash, "systemManager Soft Reset Pipeline");
        }
    });

    otaAssetUpdater.setProgressCallback([this](int percent) {
        if (auto* fwBoard = static_cast<FirmwareUpdateBoard*>(displayManager.getSystemBoard(SystemBoardId::SYS_FIRMWARE_UPDATE))) {
            fwBoard->setUpdateState(FwUpdateState::DOWNLOADING);
            fwBoard->setDownloadPercent(percent);
            displayManager.render();
        }
    });

    otaAssetUpdater.setWarningCallback([this](const char* version, int secondsRemaining) {
        if (auto* fwBoard = static_cast<FirmwareUpdateBoard*>(displayManager.getSystemBoard(SystemBoardId::SYS_FIRMWARE_UPDATE))) {
            fwBoard->setUpdateState(FwUpdateState::WARNING);
            fwBoard->setCountdownSeconds(secondsRemaining);
            fwBoard->setReleaseVersion(version);
            displayManager.showBoard(fwBoard, "OTA Firmware Update Delegate (Warning)");
            displayManager.render();
        }
    });

    otaAssetUpdater.setStateCallback([this](FwUpdateState state, const char* msg, int secondsRemaining) {
        if (auto* fwBoard = static_cast<FirmwareUpdateBoard*>(displayManager.getSystemBoard(SystemBoardId::SYS_FIRMWARE_UPDATE))) {
            fwBoard->setUpdateState(state);
            fwBoard->setCountdownSeconds(secondsRemaining);
            if (msg && msg[0] != '\0') {
                fwBoard->setErrorMessage(msg);
            }
            displayManager.showBoard(fwBoard, "OTA Firmware Update Delegate (State)");
            displayManager.render();
        }
    });

    otaAssetUpdater.setPostUpgradeCallback([this](const char* msg, int percent) {
        if (auto* load = static_cast<LoadingBoard*>(displayManager.getSystemBoard(SystemBoardId::SYS_BOOT_LOADING))) {
            load->setProgress(msg, percent);
            displayManager.showBoard(load, "OTA Firmware Update Delegate (Post-Upgrade)");
            displayManager.render();
        }
    });

    LOG_INFO("SYSTEM", "appContext initialization complete. Waiting for WiFi to start web components...");
}

/**
 * @brief Dispatch periodic maintenance ticks to all active services.
 */
void appContext::tick() {
    // 1. WiFi lifecycle (Non-blocking state machine)
    wifiManager.tick();
    yield();

    // 2. Firmware update lifecycle
    otaAssetUpdater.tick();
    yield();

    // 3. System state and refresh logic
    sysManager.tick();
    yield();

    // 4. Evaluate AppState Reactively
    if (currentState == AppState::BOOTING) {
        // Booting ends when WiFi has completed its sequence (either STA or AP)
        if (wifiManager.isReady()) {
            
            // Safe to initialize web server now that network is up
            if (!webServerInitialized) {
                LOG_INFO("SYSTEM", "Network ready. IP Address: " + WiFi.localIP().toString());
                LOG_INFO("SYSTEM", "Initializing Web Server...");
                webServer.init();

                LOG_INFO("SYSTEM", "Notifying consumers to apply initial configuration...");
                configManager.notifyConsumersToReapplyConfig(); // This calls MDNS.begin!
                
                LOG_INFO("SYSTEM", "Initializing System Clock...");
                if (auto* load = static_cast<LoadingBoard*>(displayManager.getSystemBoard(SystemBoardId::SYS_BOOT_LOADING))) {
                    load->setHeading("Departures Board");
                    load->setBuildTime(sysManager.getBuildTime().c_str());
                    int progress = 50;
                    load->setProgress("Setting the system clock...", progress);
                    displayManager.showBoard(load);

                    timeManager.initialize([&]() {
                        progress += 5;
                        if (progress > 80) progress = 45; // loop the progress bar back to avoid overflow visually
                        load->setProgress("Setting the system clock...", progress);
                        displayManager.showBoard(load);
                    });
                } else {
                    timeManager.initialize();
                }

                webServerInitialized = true;
            }

            if (!sysManager.getFirstLoad()) {
                if (wifiManager.getAPMode()) {
                    currentState = AppState::WIFI_SETUP;
                    LOG_SPLASH("APP STATE: WIFI_SETUP");
                    displayManager.showBoard(displayManager.getSystemBoard(SystemBoardId::SYS_WIFI_WIZARD));
                } else if (!configManager.hasConfiguredBoards()) {
                    currentState = AppState::BOARD_SETUP;
                    LOG_SPLASH("APP STATE: BOARD_SETUP");
                    displayManager.showBoard(displayManager.getSystemBoard(SystemBoardId::SYS_SETUP_HELP));
                } else {
                    currentState = AppState::RUNNING;
                    LOG_SPLASH("APP STATE: RUNNING");
                    displayManager.resetState();
                }
            }
        }
    } else if (wifiManager.getAPMode() && currentState != AppState::WIFI_SETUP) {
        currentState = AppState::WIFI_SETUP;
        LOG_SPLASH("APP STATE: WIFI_SETUP");
        displayManager.showBoard(displayManager.getSystemBoard(SystemBoardId::SYS_WIFI_WIZARD));
    } else if (!wifiManager.getAPMode()) {
        if (!configManager.hasConfiguredBoards() && currentState != AppState::BOARD_SETUP) {
            currentState = AppState::BOARD_SETUP;
            LOG_SPLASH("APP STATE: BOARD_SETUP");
            displayManager.showBoard(displayManager.getSystemBoard(SystemBoardId::SYS_SETUP_HELP));
        } else if (configManager.hasConfiguredBoards() && currentState != AppState::RUNNING) {
            currentState = AppState::RUNNING;
            LOG_SPLASH("APP STATE: RUNNING");
            // Allow display rotation to naturally resume
            displayManager.resetState();
        }
    }

    // 5. Display rendering tick
    displayManager.tick(millis());
    yield();

    // 6. Web server handle
    webServer.handleClient();
    yield();
}
