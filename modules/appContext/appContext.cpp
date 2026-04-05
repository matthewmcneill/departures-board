/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons
 * Attribution-NonCommercial-ShareAlike 4.0 International. To view a copy of
 * this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/appContext/appContext.cpp
 * Description: Implementation of the central appContext orchestrator.
 *
 * Exported Functions/Classes:
 * - appContext: [Class implementation]
 *   - begin: Master initialization sequence for all hardware and software services.
 *   - tick: Central administrative loop for high-level tasks and state transitions.
 *   - softResetBoard: Re-initializes the application state from flash.
 *   - getBuildTime: Utility for version reporting.
 *   - setInputDevice: Attaches a unique hardware interaction driver.
 */

#include "appContext.hpp"
#include "departuresBoard.hpp"
#include "buildTime.hpp"
#include <memory>
#include <boards/systemBoard/firmwareUpdateBoard.hpp>
#include <boards/systemBoard/loadingBoard.hpp>
#include <boards/systemBoard/splashBoard.hpp>
#include <buttonHandler.hpp>
#include <logger.hpp>
#include <wifiManager.hpp>
#include <WiFi.h>




/**
 * @brief Construct the appContext and its managed service singletons.
 */
/**
 * @brief Construct a new appContext.
 * Initializes the message pool and scheduler and sets initial state to BOOTING.
 */
appContext::appContext()
    : globalMessagePool(4), schedule(this), currentState(AppState::BOOTING),
      webServerInitialized(false), firstLoad(true), startupProgressPercent(0), 
      prevProgressBarPosition(0), inputDevice(nullptr) {
  // Note: Managers are initialized via their default constructors here.
}

/**
 * @brief Explicit destructor to handle std::unique_ptr cleanup.
 */
appContext::~appContext() = default;

/**
 * @brief Initialize all system services in the required boot order.
 */
void appContext::begin() {
  LOG_SPLASH("APP STATE: BOOTING");
  LOG_INFO("SYSTEM", "Initializing appContext managers...");

  // 0. Hardware Encryption (Must run before credentials are read)
  LOG_INFO("SYSTEM", "Initializing Cryptographic Engine...");
  deviceCrypto.begin();
  configManager.bindCrypto(&deviceCrypto);

  // 1. Hardware Display (Must be first for boot visuals/stability)
  LOG_INFO("SYSTEM", "Initializing DisplayManager...");
  displayManager.begin(this);

  // --- Cinematic Splash Sequence ---
  displayManager.setBrightness(
      0); // Guarantee screen is perfectly dark before drawing
  auto *splash = static_cast<SplashBoard *>(
      displayManager.getSystemBoard(SystemBoardId::SYS_BOOT_SPLASH));
  displayManager.showBoard(splash, "Initial Boot Logo");

  // 400ms pre-fade hold in total darkness
  delay(400);

  // 1 second fade in (51 steps * 20ms = ~1020ms)
  for (int i = 0; i <= 255; i += 5) {
    displayManager.setBrightness(i);
    delay(20);
  }
  displayManager.setBrightness(255);

  // 2 second hold
  delay(2000);

  // 1 second fade out (51 steps * 20ms = ~1020ms)
  for (int i = 255; i >= 0; i -= 5) {
    displayManager.setBrightness(i);
    delay(20);
  }

  displayManager.setBrightness(0); // Ensure perfectly dark before switching

  // 400ms post-fade hold in total darkness before transitioning
  delay(400);

  displayManager.setBrightness(
      80); // Default readable brightness before config loads
  auto *loadBoard = static_cast<LoadingBoard *>(
      displayManager.getSystemBoard(SystemBoardId::SYS_BOOT_LOADING));

  // Prepare the loadBoard header strings natively here since we want them
  // visible from 0%
  loadBoard->setHeading("Departures Board");
  loadBoard->setProgress("Initializing...", 0); 
  displayManager.showBoard(loadBoard, "Switching to Loading Screen");

  updateBootProgress(10, "Initializing hardware graphics...");

  // 2. Hardware Filesystem (Must be first for config)
  updateBootProgress(20, "Starting filesystem...");
  LittleFS.begin(true);

  // 3. Storage & Config
  updateBootProgress(30, "Loading system configuration...");
  configManager.loadApiKeys();
  configManager.loadConfig();
  const Config &config = configManager.getConfig();

  // 4. Networking (WiFi)
  updateBootProgress(40, "Initializing network services...");
  wifiManager.bindCrypto(&deviceCrypto);
  wifiManager.begin(config.hostname);

  // 5. System Management (State & Refresh)
  updateBootProgress(45, "Optimizing application state...");
  // systemManager is now dismantled, logic is integrated here or in domain managers.

#ifdef BUTTON_PIN
  // application logic
  auto btn = std::make_unique<buttonHandler>(BUTTON_PIN);
  setInputDevice(std::move(btn)); 
  LOG_INFO("SYSTEM",
           "Hardware input device attached on GPIO " + String(BUTTON_PIN));
#else
  LOG_INFO("SYSTEM", "No BUTTON_PIN defined. Hardware input disabled.");
#endif

  // Initialize Centralized Data Fetch Worker
  networkManager.init(); // Verbose logging controlled via CORE_DEBUG_LEVEL >= APP_LOG_LEVEL_VERBOSE
  networkManager.registerSource(&weather);
  networkManager.registerSource(&rss);

  LOG_INFO("SYSTEM", "Network state: SSID=" + String(WiFi.SSID()) +
                         ", IP=" + WiFi.localIP().toString());

  // 6. Register Config Consumers
  updateBootProgress(48, "Registering application services...");
  configManager.registerConsumer(&displayManager);
  configManager.registerConsumer(&weather);
  configManager.registerConsumer(&rss);
  configManager.registerConsumer(&otaAssetUpdater);
  configManager.registerConsumer(&timeManager);
  configManager.registerConsumer(&wifiManager);

  // 7. Initialize global helper modules
  otaAssetUpdater.init(this);
  timeManager.init(this);

  updateBootProgress(50, "Stabilizing application...");
  // 8. Connect Observer Callbacks for UI Decoupling
  // Migrated from systemManager: Boot progress and soft reset now handled directly within appContext.

  otaAssetUpdater.setProgressCallback([this](int percent) {
    if (auto *fwBoard =
            static_cast<FirmwareUpdateBoard *>(displayManager.getSystemBoard(
                SystemBoardId::SYS_FIRMWARE_UPDATE))) {
      fwBoard->setUpdateState(FwUpdateState::DOWNLOADING);
      fwBoard->setDownloadPercent(percent);
      displayManager.render();
    }
  });

  otaAssetUpdater.setWarningCallback([this](const char *version,
                                            int secondsRemaining) {
    if (auto *fwBoard =
            static_cast<FirmwareUpdateBoard *>(displayManager.getSystemBoard(
                SystemBoardId::SYS_FIRMWARE_UPDATE))) {
      fwBoard->setUpdateState(FwUpdateState::WARNING);
      fwBoard->setCountdownSeconds(secondsRemaining);
      fwBoard->setReleaseVersion(version);
      displayManager.showBoard(fwBoard,
                               "OTA Firmware Update Delegate (Warning)");
      displayManager.render();
    }
  });

  otaAssetUpdater.setStateCallback([this](FwUpdateState state, const char *msg,
                                          int secondsRemaining) {
    if (auto *fwBoard =
            static_cast<FirmwareUpdateBoard *>(displayManager.getSystemBoard(
                SystemBoardId::SYS_FIRMWARE_UPDATE))) {
      fwBoard->setUpdateState(state);
      fwBoard->setCountdownSeconds(secondsRemaining);
      if (msg && msg[0] != '\0') {
        fwBoard->setErrorMessage(msg);
      }
      displayManager.showBoard(fwBoard, "OTA Firmware Update Delegate (State)");
      displayManager.render();
    }
  });

  otaAssetUpdater.setPostUpgradeCallback([this](const char *msg, int percent) {
    if (auto *load = static_cast<LoadingBoard *>(
            displayManager.getSystemBoard(SystemBoardId::SYS_BOOT_LOADING))) {
      load->setProgress(msg, percent);
      displayManager.showBoard(load,
                               "OTA Firmware Update Delegate (Post-Upgrade)");
      displayManager.render();
    }
  });

  // 10. Initialize the Scheduler
  schedule.begin();

  LOG_INFO(
      "SYSTEM",
      "🔘 [SYSTEM] appContext initialization complete. Waiting for WiFi...");
}

/**
 * @brief Update the loading sequence visual progress output.
 */
void appContext::updateBootProgress(int percentage, const char *msg) {
  LOG_INFO("SYSTEM", msg);
  if (auto *load = static_cast<LoadingBoard *>(
          displayManager.getSystemBoard(SystemBoardId::SYS_BOOT_LOADING))) {
    load->setProgress(msg, percentage, 600);
    displayManager.showBoard(load, "Boot Progress Update", 600);
  }
}

/**
 * @brief Dispatch periodic maintenance ticks to all active services.
 */
void appContext::tick() {
  // 0. Process Deferred Configuration Reloads
  if (configManager.checkAndClearReload()) {
    LOG_INFO("SYSTEM", "Executing deferred configuration reload safely on system thread...");
    configManager.loadConfig();
    configManager.notifyConsumersToReapplyConfig();
  }

  // 1. WiFi lifecycle (Non-blocking state machine)
  wifiManager.tick();
  yield();

  // 2. Firmware update lifecycle
  otaAssetUpdater.tick();
  yield();

  // 3. System state and refresh logic
  // Migrated: sysManager.tick() logic is now integrated or handle by domain managers  // 4. Evaluate AppState Reactively
  if (currentState == AppState::BOOTING) {
    // Phase 2: Wait for WiFi (50-65%)
    if (!wifiManager.isReady()) {
      static uint32_t lastWifiProgressUpdate = 0;
      if (millis() - lastWifiProgressUpdate > 1000) {
        lastWifiProgressUpdate = millis();
        // Slowly increment progress from 50 to 65 while waiting
        static int wifiProgress = 50;
        if (wifiProgress < 65) wifiProgress++;
        updateBootProgress(wifiProgress, "Connecting to network...");
      }
    } else {
      // WiFi is ready. 
      // Phase 3: Web Server & NTP (65-80%)
      if (!webServerInitialized) {
        LOG_INFO("SYSTEM",
                 "Network ready. IP Address: " + WiFi.localIP().toString());
        LOG_INFO("SYSTEM", "Initializing Web Server...");
        webServer.init();

        LOG_INFO("SYSTEM", "Applying critical system configuration...");
        displayManager.applyConfig(configManager.getConfig());
        timeManager.reapplyConfig(configManager.getConfig());
        wifiManager.reapplyConfig(configManager.getConfig());

        LOG_INFO("SYSTEM", "Initializing System Clock...");
        if (auto *load =
                static_cast<LoadingBoard *>(displayManager.getSystemBoard(
                    SystemBoardId::SYS_BOOT_LOADING))) {
          load->setHeading("Departures Board");
          load->setBuildTime(getBuildTime().c_str());
        }

        int ntpProgress = 65;
        updateBootProgress(ntpProgress, "Setting the system clock...");

        timeManager.initialize([&]() {
          if (ntpProgress < 80) ntpProgress += 2;
          updateBootProgress(ntpProgress, "Setting the system clock...");
        });

        webServerInitialized = true;
      }

      // Phase 4: Wait for First Data (80-100%)
      if (firstLoad) {
        // Force a low-level fetch check to advance progress
        if (!configManager.hasConfiguredBoards()) {
           // No boards configured? Transition instantly.
           firstLoad = false;
        } else {
           // Progress moves as data comes in
           if (!networkManager.getNoDataLoaded()) {
              updateBootProgress(100, "Finalizing UI...");
              delay(300); // Brief pause at 100% for visual satisfaction
              firstLoad = false;
           } else {
              // Determine progress to 100 based on some heuristic or just stay at 80
              updateBootProgress(80, "Fetching station data...");
           }
        }
      }

      if (!firstLoad) {
        if (wifiManager.getAPMode()) {
          currentState = AppState::WIFI_SETUP;
          LOG_SPLASH("APP STATE: WIFI_SETUP");
          displayManager.showBoard(
              displayManager.getSystemBoard(SystemBoardId::SYS_WIFI_WIZARD));
        } else if (!configManager.hasConfiguredBoards()) {
          currentState = AppState::BOARD_SETUP;
          LOG_SPLASH("APP STATE: BOARD_SETUP");
          displayManager.showBoard(
              displayManager.getSystemBoard(SystemBoardId::SYS_SETUP_HELP));
        } else {
          currentState = AppState::RUNNING;
          LOG_SPLASH("APP STATE: RUNNING");

          // Trigger global data refresh and peripheral provisioning now that
          // system is stable
          configManager.notifyConsumersToReapplyConfig();

          // Let the display manager resume, which now consults the scheduler
          displayManager.resumeDisplays();
        }
      }
    }
  } else if (wifiManager.getAPMode() && currentState != AppState::WIFI_SETUP) {
    currentState = AppState::WIFI_SETUP;
    LOG_SPLASH("APP STATE: WIFI_SETUP");
    displayManager.showBoard(
        displayManager.getSystemBoard(SystemBoardId::SYS_WIFI_WIZARD));
  } else if (!wifiManager.getAPMode()) {
    if (!configManager.hasConfiguredBoards() &&
        currentState != AppState::BOARD_SETUP) {
      currentState = AppState::BOARD_SETUP;
      LOG_SPLASH("APP STATE: BOARD_SETUP");
      displayManager.showBoard(
          displayManager.getSystemBoard(SystemBoardId::SYS_SETUP_HELP));
    } else if (configManager.hasConfiguredBoards() &&
               currentState != AppState::RUNNING) {
      currentState = AppState::RUNNING;
      LOG_SPLASH("APP STATE: RUNNING");
      // Allow display rotation to naturally resume
      displayManager.resumeDisplays();
    }
  }

  // 5. Display rendering tick
  displayManager.tick(millis());
  yield();

  // 6. Web server handle (Deprecated: ESPAsyncWebServer handles background
  // automatically)
  // yield();

  // 7. Input Device Update
  if (inputDevice) {
    inputDevice->update();
    if (inputDevice->wasShortTapped() || inputDevice->wasLongTapped()) {
      // Any hardware interaction triggers a manual override in the scheduler,
      // ensuring we stay awake for the configured timeout.
      bool wasSleeping = (displayManager.getActiveBoardType() == BoardType::CLOCK_BOARD);
      schedule.triggerManualOverride();

      if (wasSleeping) {
        LOG_INFO("INPUT", "Hardware button tapped: Waking from sleep/screensaver");
        displayManager.resumeDisplays();
      } else if (inputDevice->wasShortTapped()) {
        LOG_INFO("INPUT", "Short tap: Cycling next board");
        displayManager.cycleNext();
      } else {
        LOG_INFO("INPUT", "Long tap: Manual override refreshed");
      }
    }
  }

  // 8. Connection Management
  // Migrated logic from systemManager for tracking WiFi status changes.
  static bool wifiConnectedCached = false;
  bool wifiConnectedNow = (WiFi.status() == WL_CONNECTED);
  if (!wifiConnectedNow && wifiConnectedCached) {
    wifiConnectedCached = false;
    LOG_WARN("SYSTEM", "WiFi connection lost.");
  } else if (wifiConnectedNow && !wifiConnectedCached) {
    wifiConnectedCached = true;
    LOG_INFO("SYSTEM", "WiFi connected. IP: " + WiFi.localIP().toString());
  }
}

/**
 * @brief Performs a soft reload of the application state based on new configuration.
 */
void appContext::softResetBoard() {
  const Config& config = configManager.getConfig();
  
  LOG_INFO("SYSTEM", "Performing soft reset...");
  configManager.loadConfig();
  displayManager.applyConfig(config);
  
  // Timezone sync
  if (timeManager.getTimezone() != "") {
    setenv("TZ", timeManager.getTimezone().c_str(), 1);
  } else {
    setenv("TZ", TimeManager::ukTimezone, 1);
  }
  tzset();

  currentState = AppState::BOOTING;
  if (auto *splash = static_cast<SplashBoard *>(
          displayManager.getSystemBoard(SystemBoardId::SYS_BOOT_SPLASH))) {
    displayManager.showBoard(splash, "Soft Reset Pipeline");
  }

  displayManager.resumeDisplays();
  firstLoad = true;
  startupProgressPercent = 70;
  prevProgressBarPosition = 70;

  globalMessagePool.clear();
}

/**
 * @brief Get the Build Timestamp of the running firmware
 */
String appContext::getBuildTime() {
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
 * @brief Attach a hardware interaction driver.
 * @param device Unique pointer to a buttonHandler implementation.
 */
void appContext::setInputDevice(std::unique_ptr<buttonHandler> device) {
  inputDevice = std::move(device);
}
