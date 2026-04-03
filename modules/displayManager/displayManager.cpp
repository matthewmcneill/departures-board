#include <departuresBoard.hpp>
#include <fonts/fonts.hpp>
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
 * Module: modules/displayManager/displayManager.cpp
 * Description: Implementation of the DisplayManager orchestrator. Handles
 *              the lifecycle of hardware displays, including SPI
 * initialization, board rendering, memory pool allocation, and power management
 *              via a scheduled sleep system.
 *
 * Exported Functions/Classes:
 * - DisplayManager::DisplayManager: Constructor initializing default state.
 * - DisplayManager::begin: Initialize hardware display driver.
 * - DisplayManager::tick: Executive rendering and state management loop.
 * - DisplayManager::render: Forces an immediate synchronized display refresh.
 * - DisplayManager::showBoard: Switch to a new board and optionally block for
 * an animation duration.
 * - DisplayManager::applyConfig: Provisions system state from a central config
 * object.
 * - DisplayManager::setPowerSave: Controls the hardware OLED power state.
 * - DisplayManager::resumeDisplays: Wakes the display and returns to the
 * carousel.
 * - DisplayManager::cycleNext: Advances the active board in the carousel.
 * - DisplayManager::getSystemBoard: Retrieve a system board with lazy-allocation.
 * - DisplayManager::setBrightness: Hardware OLED intensity control.
 * - DisplayManager::setFlipScreen: 180-degree rotation control.
 */

#include "boardFactory.hpp"
#include "buildTime.hpp"
#include "configManager.hpp"
#include "displayManager.hpp"
#include <algorithm>
#include <appContext.hpp>
#include <logger.hpp>
#include <otaUpdater.hpp>
#include <rssClient.hpp>
#include <vector>
#include <weatherClient.hpp>

// We must include Board specialized headers internally within the cpp only when
// absolutely needed to invoke methods that the generic iDisplayBoard does not
// have (like setHeading).
#include "boards/busBoard/busBoard.hpp"
#include "boards/nationalRailBoard/nationalRailBoard.hpp"
#include "boards/systemBoard/helpBoard.hpp"
#include "boards/systemBoard/loadingBoard.hpp"
#include "boards/systemBoard/messageBoard.hpp"
#include "boards/systemBoard/sleepingBoard.hpp"
#include "boards/systemBoard/splashBoard.hpp"
#include "boards/systemBoard/wizardBoard.hpp"
#include "boards/tflBoard/tflBoard.hpp"

// DisplayManager singleton will be owned by appContext

/** @return True if screen is flipped 180 degrees. */
bool DisplayManager::getFlipScreen() const { return flipScreen; }
/** @return True if scheduled sleep is enabled. */
bool DisplayManager::getSleepEnabled() const { return sleepEnabled; }
/** @brief Enable or disable automated sleep schedule. */
void DisplayManager::setSleepEnabled(bool enabled) { sleepEnabled = enabled; }
/** @return Hour (0-23) when sleep starts. */
byte DisplayManager::getSleepStarts() const { return sleepStarts; }
/** @brief Set the hour to start sleep mode. */
void DisplayManager::setSleepStarts(byte starts) { sleepStarts = starts; }
/** @return Hour (0-23) when sleep ends. */
byte DisplayManager::getSleepEnds() const { return sleepEnds; }
/** @brief Set the hour to end sleep mode. */
void DisplayManager::setSleepEnds(byte ends) { sleepEnds = ends; }
/** @return True if sleep is manually forced. */
bool DisplayManager::getForcedSleep() const { return forcedSleep; }
/** @brief Manually force the display into sleep mode. */
void DisplayManager::setForcedSleep(bool active) { forcedSleep = active; }

// --- Singleton Instance ---
// This will be removed in favor of appContext ownership.
// DisplayManager displayManager; // Global system display orchestrator

// --- Lifecycle Methods ---

/**
 * @brief Default constructor. Pre-configures the u8g2 hardware driver with SPI
 * pins.
 */
DisplayManager::DisplayManager()
    : u8g2(U8G2_R0, /* cs=*/DISPLAY_CS_PIN, /* dc=*/DISPLAY_DC_PIN,
           /* reset=*/U8X8_PIN_NONE),
      activeSlotIndex(0), splashBoard(nullptr), loadingBoard(nullptr),
      wizardBoard(nullptr), helpBoard(nullptr), messageBoard(nullptr),
      firmwareUpdateBoard(nullptr), sleepingBoard(nullptr),
      diagnosticBoard(nullptr), brightness(20), flipScreen(false),
      sleepEnabled(false), sleepStarts(23), sleepEnds(8), currentBoard(nullptr),
      lastActivity(0), forcedSleep(false), wifiWarning(0, 56) {
  for (int i = 0; i < MAX_BOARDS; i++) {
    slots[i] = nullptr;
  }
}

/**
 * @brief Initialize the hardware display and apply initial buffer settings.
 * @param contextPtr Pointer to the parent application context.
 */
void DisplayManager::begin(appContext *contextPtr) {
  context = contextPtr;

  // Initialization remains in begin() as we need u8g2 hardware started
  u8g2.begin();
  u8g2.setDrawColor(1);
  u8g2.setFontMode(1);
  u8g2.setFontRefHeightAll();
  u8g2.setFontPosTop();
  u8g2.setFont(NatRailTall12);

  // Lazy-load the first screen (Splash) via getSystemBoard
  iDisplayBoard *initialBoard = getSystemBoard(SystemBoardId::SYS_BOOT_SPLASH);
  if (initialBoard)
    showBoard(initialBoard, "DisplayManager::begin() Hardware Initialization");

  lastActivity = millis();
}

/**
 * @brief Top-level execution tick. Handles sleep state transitions,
 *        board logic updates, and triggers rendering.
 * @param currentMillis Current system uptime.
 */
void DisplayManager::tick(unsigned long currentMillis) {
  // --- Step 1: Handle Sleep Transitions ---
  // Determine if the screen should be dimmed/clocked based on schedule or
  // force-sleep.
  bool shouldSnooze = isSnoozing();
  iDisplayBoard *sleepBoard = getSystemBoard(SystemBoardId::SYS_SLEEP_CLOCK);

  if (shouldSnooze && currentBoard != sleepBoard && sleepBoard != nullptr) {
    // Transition TO Sleep: Dim the OLED and swap active board to the clock.
    showBoard(sleepBoard, "Screensaver sleep schedule triggered");
    u8g2.setContrast(((SleepingBoard *)sleepBoard)->getDimmedBrightness());
  } else if (!shouldSnooze && currentBoard == sleepBoard) {
    // Transition FROM Sleep: Restore user brightness and the last carousel
    // board.
    showBoard(getDisplayBoard(activeSlotIndex), "Waking up from screensaver");
    u8g2.setContrast(brightness);
    u8g2.clearDisplay(); // Ensure clean surface for data rendering
  }

  // --- Step 2: Adaptive Schedule Check ---
  // If we're not snoozing and system is running, periodically ensure our
  // current board is still valid.
  if (!shouldSnooze && context->getAppState() == AppState::RUNNING &&
      currentMillis - lastScheduleCheck > 3000UL) {
    lastScheduleCheck = currentMillis;
    std::vector<int> activeBoards =
        context->getSchedulerManager().getActiveBoards();
    if (!activeBoards.empty()) {
      bool currentAllowed = false;
      for (int idx : activeBoards) {
        if (idx == activeSlotIndex) {
          currentAllowed = true;
          break;
        }
      }
      if (!currentAllowed) {
        LOG_INFO("DISPLAY", "Current board no longer in active schedule. "
                            "Transitioning to next allowed...");
        cycleNext();
      }
    }
  }

  // --- Step 3: Global Data Source Synchronization ---
  // Poll each active UI board periodically so they can sync their layout state with background DataManager fetches
  static unsigned long lastDataSync = 0;
  if (currentMillis - lastDataSync >= 500) {
    lastDataSync = currentMillis;
    for (int i = 0; i < MAX_BOARDS; i++) {
        if (slots[i]) slots[i]->updateData();
    }
  }

  // --- Step 4: Internal Logic and Rendering ---
  // Delegate state updates to the active board.
  if (currentBoard != nullptr)
    currentBoard->tick(currentMillis);

  // --- Step 3: Carousel Auto-Rotation Pacing ---
  // rotation only occurs in Running state and when showing a carousel board
  // (not a system overlay)
  bool isCarouselBoard = (currentBoard == getDisplayBoard(activeSlotIndex));

  if (!shouldSnooze && context->getAppState() == AppState::RUNNING &&
      isCarouselBoard && !diagModeActive && context != nullptr) {
    const Config &config = context->getConfigManager().getConfig();
    if (config.boardCount > 1) {
      // Only rotate if the interval has passed
      if (currentMillis - lastActivity >
          (config.carouselIntervalSecs * 1000UL)) {
        // Defer rotation if board is still animating and setting requires it
        if (!waitForScrollComplete || currentBoard->isScrollFinished()) {
          cycleNext(); // Resets lastActivity internally
        }
      }
    }
  }

  // Trigger the actual hardware buffer transaction.
  render();
}

void DisplayManager::setDiagMode(bool active) {
  if (diagModeActive != active) {
    diagModeActive = active;
    if (diagModeActive) {
      iDisplayBoard *diagBoard = getSystemBoard(SystemBoardId::SYS_DIAGNOSTIC);
      if (diagBoard)
        showBoard(diagBoard, "Diagnostic Mode Activated (Volatile)");
    } else {
      // Restore current carousel slot
      showBoard(getDisplayBoard(activeSlotIndex),
                "Diagnostic Mode Deactivated");
      u8g2.clearDisplay();
    }
  }
}

/**
 * @brief Perform a full synchronized render transaction.
 * @note Manages clearBuffer(), rendering the active/override board, and
 * sendBuffer().
 */
void DisplayManager::render() {
  // --- Step 1: Prepare frame buffer ---
  u8g2.clearBuffer();

  // --- Step 2: Delegate board rendering ---
  if (currentBoard != nullptr) {
    currentBoard->render(u8g2);
  }

  // --- Step 3: Send to display ---

  // --- Step 4: Hardware push ---
  u8g2.sendBuffer();
}

iDisplayBoard *DisplayManager::getCurrentBoard() { return currentBoard; }

/**
 * @brief Set the hardware power save state (OLED on/off) and track it
 * internally.
 * @param off True to power down the OLED, false to wake it up.
 */
void DisplayManager::setPowerSave(bool off) {
  if (oledPowerSaveActive != off) {
    oledPowerSaveActive = off;
    u8g2.setPowerSave(off ? 1 : 0);
  }
}

/**
 * @brief Switch to a new board and optionally block for an animation duration.
 * @param board Pointer to the board implementation to render.
 * @param durationMs Optional duration in ms to block and animate.
 */
void DisplayManager::showBoard(iDisplayBoard *board, const char *reason,
                               uint32_t durationMs) {
  if (currentBoard != board) {
    // --- Step 0: Power Management Safety ---
    // Ensure the display is active before switching to a new board.
    // This prevents the screen from being stuck "off" if a board deactivated
    // while in power-save mode.
    setPowerSave(false);

    // Log the transition for hardware-side diagnostics
    String msg = "showBoard() invoked for: [" +
                 String(board ? board->getBoardName() : "NULL") +
                 "] | Reason: " + String(reason);
    LOG_INFO("DISPLAY", msg.c_str());

    // De-initialize the departing board
    if (currentBoard != nullptr) {
      currentBoard->onDeactivate();
    }

    // Switch context
    currentBoard = board;

    // Initialize the new board
    if (currentBoard != nullptr) {
      currentBoard->onActivate();
      LOG_INFO("DISPLAY", "Board activated.");
    }

    // Push initial frame immediately to prevent black screen during transit
    render();
  }

  // --- Step 2: Blocking Animation Support ---
  if (durationMs > 0) {
    unsigned long end = millis() + durationMs;
    // Run a local rendering loop for the specified duration.
    // Capped at ~60 FPS to prevent OLED SPI DMA saturation.
    while (millis() < end) {
      tick(millis());
      delay(15); // Yield to RTOS and feed watchdog
    }
  }
}

// --- Configuration Accessors ---

/**
 * @brief Sets the global display brightness.
 * @param level Hardware level (0-255).
 */
void DisplayManager::setBrightness(int level) {
  brightness = level;
  u8g2.setContrast(brightness);
}

/**
 * @brief Retrieve a functional system board singleton from the registry.
 * @param id The identifier for the required screen.
 * @return iDisplayBoard* Pointer to the configured singleton.
 */
iDisplayBoard *DisplayManager::getSystemBoard(SystemBoardId id) {
  switch (id) {
  case SystemBoardId::SYS_BOOT_SPLASH:
    if (!splashBoard) {
      splashBoard =
          BoardFactory::createSystemBoard(static_cast<int>(id), context);
      if (splashBoard)
        ((SplashBoard *)splashBoard)->init(context);
    }
    return splashBoard;

  case SystemBoardId::SYS_BOOT_LOADING:
    if (!loadingBoard) {
      loadingBoard =
          BoardFactory::createSystemBoard(static_cast<int>(id), context);
      if (loadingBoard)
        ((LoadingBoard *)loadingBoard)->init(context);
    }
    if (loadingBoard) {
      ((LoadingBoard *)loadingBoard)->setHeading("Departures Board");
#ifdef BUILD_TIME
      ((LoadingBoard *)loadingBoard)->setBuildTime(BUILD_TIME);
#else
      ((LoadingBoard *)loadingBoard)
          ->setBuildTime(String(__DATE__ " " __TIME__).c_str());
#endif
    }
    return loadingBoard;

  case SystemBoardId::SYS_WIFI_WIZARD:
    if (!wizardBoard) {
      wizardBoard =
          BoardFactory::createSystemBoard(static_cast<int>(id), context);
      if (wizardBoard)
        ((WizardBoard *)wizardBoard)->init(context);
    }
    if (wizardBoard)
      ((WizardBoard *)wizardBoard)->setWizardIp(WiFi.softAPIP());
    return wizardBoard;

  case SystemBoardId::SYS_SETUP_HELP: {
    if (!helpBoard) {
      helpBoard =
          BoardFactory::createSystemBoard(static_cast<int>(id), context);
      if (helpBoard)
        ((HelpBoard *)helpBoard)->init(context);
    }
    char ipStr[32];
    snprintf(ipStr, sizeof(ipStr), "Browse: http://%s",
             WiFi.localIP().toString().c_str());
    const char *lines[] = {"To configure this display,",
                           "connect to the same WiFi",
                           "and navigate to:", ipStr, "Settings -> Displays"};
    if (helpBoard)
      ((HelpBoard *)helpBoard)->setHelpContent("Setup Guide", lines, 5);
    return helpBoard;
  }

  case SystemBoardId::SYS_ERROR_NO_DATA:
  case SystemBoardId::SYS_ERROR_WSDL:
  case SystemBoardId::SYS_ERROR_TOKEN:
  case SystemBoardId::SYS_ERROR_CRS:
  case SystemBoardId::SYS_ERROR_WIFI:
    if (!messageBoard) {
      messageBoard =
          BoardFactory::createSystemBoard(static_cast<int>(id), context);
      if (messageBoard)
        ((MessageBoard *)messageBoard)->init(context);
    }

    if (messageBoard) {
      if (id == SystemBoardId::SYS_ERROR_NO_DATA) {
        ((MessageBoard *)messageBoard)
            ->setContent("** COMMS ERROR **", "NO DEPARTURE DATA",
                         "UNABLE TO GET DATA FOR", "STATION.");
      } else if (id == SystemBoardId::SYS_ERROR_WSDL) {
        ((MessageBoard *)messageBoard)
            ->setContent("** WSDL ERROR **", "FAILED TO INIT API",
                         "CANNOT CONTACT NATIONAL", "RAIL SERVERS.");
      } else if (id == SystemBoardId::SYS_ERROR_TOKEN) {
        ((MessageBoard *)messageBoard)
            ->setContent("** AUTHORISATION **", "INVALID API KEY",
                         "CHECK YOUR TOKEN AND", "TFL CREDENTIALS VIA",
                         "THE WEB SETUP WIZARD.");
      } else if (id == SystemBoardId::SYS_ERROR_CRS) {
        ((MessageBoard *)messageBoard)
            ->setContent("** STATION ERROR **", "INVALID CRS/NAPTAN",
                         "STATION CODE NOT", "RECOGNISED BY API.");
      } else if (id == SystemBoardId::SYS_ERROR_WIFI) {
        char ipStr[32];
        snprintf(ipStr, sizeof(ipStr), "RETRY: http://%s",
                 WiFi.localIP().toString().c_str());
        ((MessageBoard *)messageBoard)
            ->setContent("** WIFI ERROR **", "CONNECTION LOST", ipStr,
                         "CHECK YOUR ROUTER.");
      }
    }
    return messageBoard;

  case SystemBoardId::SYS_FIRMWARE_UPDATE:
    if (!firmwareUpdateBoard) {
      firmwareUpdateBoard =
          BoardFactory::createSystemBoard(static_cast<int>(id), context);
      // FirmwareUpdateBoard doesn't need explicit init(context) beyond Factory
      // creation
    }
    return firmwareUpdateBoard;

  case SystemBoardId::SYS_SLEEP_CLOCK:
    if (!sleepingBoard) {
      sleepingBoard =
          BoardFactory::createSystemBoard(static_cast<int>(id), context);
    }
    return sleepingBoard;

  case SystemBoardId::SYS_DIAGNOSTIC:
    if (!diagnosticBoard) {
      diagnosticBoard =
          BoardFactory::createSystemBoard(static_cast<int>(id), context);
    }
    return diagnosticBoard;

  default:
    return nullptr;
  }
}

/**
 * @brief Maps an interface error status (UpdateStatus) to a functional
 * SystemBoardId.
 */
SystemBoardId DisplayManager::mapErrorToId(UpdateStatus status) {
  switch (status) {
  case UpdateStatus::UNAUTHORISED:
    return SystemBoardId::SYS_ERROR_TOKEN;
  case UpdateStatus::DATA_ERROR:
  case UpdateStatus::TIMEOUT:
  case UpdateStatus::HTTP_ERROR:
    return SystemBoardId::SYS_ERROR_NO_DATA;
  case UpdateStatus::NO_RESPONSE:
    return SystemBoardId::SYS_ERROR_WSDL;
  case UpdateStatus::INCOMPLETE:
    return SystemBoardId::SYS_ERROR_CRS;
  default:
    return SystemBoardId::SYS_ERROR_NO_DATA;
  }
}

/**
 * @brief Sets the screen flip mode.
 * @param flip True to flip the screen, false otherwise.
 */
void DisplayManager::setFlipScreen(bool flip) {
  flipScreen = flip;
  u8g2.setFlipMode(flipScreen ? 1 : 0);
}

/**
 * @brief Access the system-wide global message pool.
 * @return MessagePool* Pointer to the global pool.
 */
MessagePool *DisplayManager::getGlobalMessagePool() {
  return (context != nullptr) ? &context->getGlobalMessagePool() : nullptr;
}

/**
 * @brief Checks if the system is currently in a sleep state.
 * @return True if the current board is the sleep clock.
 */
bool DisplayManager::getIsSleeping() const {
  return (currentBoard != nullptr &&
          currentBoard == const_cast<DisplayManager *>(this)->getSystemBoard(
                              SystemBoardId::SYS_SLEEP_CLOCK));
}

/**
 * @brief Checks if the display should be snoozing based on manual override or
 * schedule.
 * @return True if system should transition to sleep.
 */
bool DisplayManager::isSnoozing() {
  // --- Step 1: Explicit overrides ---
  if (forcedSleep)
    return true;
  if (!sleepEnabled)
    return false;

  // --- Step 2: Schedule evaluation ---
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
    return false; // Time sync required for scheduling

  byte myHour = timeinfo.tm_hour;
  if (sleepStarts > sleepEnds) {
    // Range spans midnight (e.g. 23:00 to 07:00)
    if ((myHour >= sleepStarts) || (myHour < sleepEnds))
      return true;
    else
      return false;
  } else {
    // Standard range within one day (e.g. 10:00 to 14:00)
    if ((myHour >= sleepStarts) && (myHour < sleepEnds))
      return true;
    else
      return false;
  }
}

// --- Board Management ---

/**
 * @brief Retrieve a pointer to a board in the carousel pool.
 * @param slotIndex Index (0 to MAX_BOARDS-1).
 * @return iDisplayBoard* Pointer to active implementation, or nullptr if empty.
 */
iDisplayBoard *DisplayManager::getDisplayBoard(int slotIndex) {
  if (slotIndex < 0 || slotIndex >= MAX_BOARDS)
    return nullptr;
  return slots[slotIndex];
}

BoardType DisplayManager::getActiveBoardType() {
  iDisplayBoard *current = slots[activeSlotIndex];
  if (current == nullptr)
    return BoardType::NR_BOARD;

  // We can infer type by RTTI or string matching, but for now we look at the
  // name as dynamic_cast is disabled on our embedded platform due to -fno-rtti
  const char *boardName = current->getBoardName();

  if (strstr(boardName, "National Rail") != nullptr)
    return BoardType::NR_BOARD;
  if (strstr(boardName, "TfL Board") != nullptr)
    return BoardType::TFL_BOARD;
  if (strstr(boardName, "Bus Board") != nullptr)
    return BoardType::BUS_BOARD;
  if (strstr(boardName, "Sleep Clock") != nullptr)
    return BoardType::CLOCK_BOARD;
  if (strstr(boardName, "Diagnostics") != nullptr)
    return BoardType::DIAGNOSTIC_BOARD;

  return BoardType::NR_BOARD;
}

/**
 * @brief Allocates a specific Board type into a designated Carousel slot.
 * @param slotIndex Index (0 to MAX_BOARDS-1).
 * @param type Board implementation type to instantiate.
 */
void DisplayManager::setBoardType(int slotIndex, BoardType type) {
  if (slotIndex < 0 || slotIndex >= MAX_BOARDS)
    return;

  clearSlot(slotIndex); // Clean up any existing board before making a new one

  slots[slotIndex] =
      BoardFactory::createDisplayBoard(static_cast<int>(type), context);
}

void DisplayManager::clearSlot(int slotIndex) {
  if (slotIndex >= 0 && slotIndex < MAX_BOARDS) {
    if (slots[slotIndex] != nullptr) {
      delete slots[slotIndex];
      slots[slotIndex] = nullptr;
    }
  }
}

iDisplayBoard **DisplayManager::getSlot(int slotIndex) {
  if (slotIndex >= 0 && slotIndex < MAX_BOARDS)
    return &slots[slotIndex];
  return nullptr;
}

/**
 * @brief Free all carousel slots and reset to uninitialized state.
 */
void DisplayManager::clearSlots() {
  for (int i = 0; i < MAX_BOARDS; i++) {
    clearSlot(i);
  }
}

/**
 * @brief Add a board of the specified type to the first available slot.
 * @param type Board implementation type.
 * @return iDisplayBoard* Pointer to the new board, or nullptr if full.
 */
iDisplayBoard *DisplayManager::addBoard(BoardType type) {
  for (int i = 0; i < MAX_BOARDS; i++) {
    if (slots[i] == nullptr) {
      setBoardType(i, type);
      return getDisplayBoard(i);
    }
  }
  return nullptr;
}

/**
 * @brief Rotates the carousel index to the next scheduled board.
 * @return The new active slot index.
 */
int DisplayManager::cycleNext() {
  if (!context)
    return activeSlotIndex;

  std::vector<int> activeBoards =
      context->getSchedulerManager().getActiveBoards();
  if (activeBoards.empty())
    return activeSlotIndex; // Subsystem failed or list is empty

  lastActivity = millis(); // Reset the auto-rotation pacing timer

  // Find our current rotational position within the active subset
  auto it =
      std::find(activeBoards.begin(), activeBoards.end(), activeSlotIndex);

  int nextSlotIndex = activeSlotIndex;
  if (it != activeBoards.end()) {
    auto nextIt = std::next(it);
    if (nextIt == activeBoards.end()) {
      // Reached the end of the subset, wrap back to the first active board
      nextSlotIndex = activeBoards.front();
    } else {
      nextSlotIndex = *nextIt;
    }
  } else {
    // Current board is no longer in the active schedule, snap to the first
    // valid one
    nextSlotIndex = activeBoards.front();
  }

  // Do not unnecessarily trigger an animation cycle if there is only 1 active
  // board
  if (nextSlotIndex == activeSlotIndex && activeBoards.size() == 1) {
    return activeSlotIndex;
  }

  activeSlotIndex = nextSlotIndex;
  iDisplayBoard *board = getDisplayBoard(activeSlotIndex);
  if (board != nullptr) {
    showBoard(board, "Carousel Auto-Rotation or Manual Skip");
  }
  return activeSlotIndex;
}

// --- Rendering Helpers ---

/**
 * @brief Logic scroller that provides sub-ms timing to boards for scrolling
 * animations. Capped at ~60fps (16ms per update).
 */
void DisplayManager::yieldAnimationUpdate() {
  static uint32_t lastAnim = 0;
  uint32_t now = millis();

  // Enforce a strict frame timing of ~60fps (16ms)
  if (now - lastAnim >= 16) {
    lastAnim = now;

    // Update delegated animation logic for board
    if (currentBoard)
      currentBoard->renderAnimationUpdate(u8g2, now);

    // --- Critical Step: Background Tasking ---
    // Allow the web server to process requests during long-duration
    // network fetches or intensive scrolling.
    // (Deprecated: ESPAsyncWebServer handles background automatically)
  }
}

/**
 * @brief Provisions the hardware display and carousel based on a central
 * config.
 * @param config Reference to the source configuration struct.
 */
void DisplayManager::applyConfig(const Config &config) {
  LOG_INFO("DISPLAY", "Applying configuration to DisplayManager...");

  // Save previous board and detach to force fresh onActivate
  iDisplayBoard *previousBoard = currentBoard;
  currentBoard = nullptr;

  // --- Step 1: Apply Global Hardware and Power settings ---
  setBrightness(config.brightness);
  setFlipScreen(config.flipScreen);
  setSleepEnabled(config.sleepEnabled);
  setSleepStarts((byte)config.sleepStarts);
  setSleepEnds((byte)config.sleepEnds);

  // Save pacing/power flags for tick()
  waitForScrollComplete = config.waitForScrollComplete;
  prioritiseRss = config.prioritiseRss;

  // Sync extra attributes to specialized system boards
  SleepingBoard *sb =
      (SleepingBoard *)getSystemBoard(SystemBoardId::SYS_SLEEP_CLOCK);
  if (sb)
    sb->setShowClock(config.showClockInSleep);

  // Scan config for the first Screensaver board and use its OLED-off setting
  // for the system sleep screen
  for (int i = 0; i < config.boardCount; i++) {
    if (config.boards[i].type == MODE_CLOCK) {
      if (sb)
        sb->setOledOff(config.boards[i].oledOff);
      break;
    }
  }

  // --- Step 2: Clear and Re-provision Carousel Slots ---
  clearSlots();

  for (int i = 0; i < config.boardCount; i++) {
    const BoardConfig &bc = config.boards[i];
    LOG_INFO("DISPLAY", String("Provisioning Board ") + i +
                            " (Type: " + (int)bc.type + ")");

    switch (bc.type) {
    case MODE_RAIL: {
      setBoardType(i, BoardType::NR_BOARD);
      NationalRailBoard *rb = (NationalRailBoard *)getDisplayBoard(i);
      if (rb) {
        rb->setCrsCode(bc.id);
        rb->setStationLat(bc.lat);
        rb->setStationLon(bc.lon);
        rb->setCallingCrsCode(bc.secondaryId);
        rb->setCallingStation(bc.secondaryName);
        rb->setPlatformFilter(bc.filter);
        rb->setNrTimeOffset(bc.timeOffset);
      }
      break;
    }
    case MODE_TUBE: {
      setBoardType(i, BoardType::TFL_BOARD);
      TfLBoard *tb = (TfLBoard *)getDisplayBoard(i);
      if (tb) {
        tb->setTubeId(bc.id);
        tb->setTubeName(bc.name);
      }
      break;
    }
    case MODE_BUS: {
      setBoardType(i, BoardType::BUS_BOARD);
      BusBoard *bb = (BusBoard *)getDisplayBoard(i);
      if (bb) {
        bb->setBusAtco(bc.id);
        bb->setBusName(bc.name);
        bb->setBusLat(bc.lat);
        bb->setBusLon(bc.lon);
        bb->setBusFilter(bc.filter);
      }
      break;
    }
    case MODE_CLOCK: {
      setBoardType(i, BoardType::CLOCK_BOARD);
      break;
    }
    case MODE_DIAGNOSTIC: {
      setBoardType(i, BoardType::DIAGNOSTIC_BOARD);
      break;
    }
    }

    iDisplayBoard *dispBoard = getDisplayBoard(i);
    if (dispBoard)
      dispBoard->configure(bc);
  }

  // --- Step 3: Finalize Initial View ---
  if (context && context->getAppState() != AppState::RUNNING) {
    // Do not activate configured display boards if we aren't in RUNNING state
    // yet. Restore whatever system board was overriding the screen.
    if (previousBoard != nullptr) {
      showBoard(previousBoard, "Context Restore: Reverting to previous slot");
    } else {
      iDisplayBoard *splash = getSystemBoard(SystemBoardId::SYS_BOOT_SPLASH);
      if (splash)
        showBoard(splash,
                  "Context Restore fallback: No boards left in carousel");
    }
  } else if (previousBoard != getSystemBoard(SystemBoardId::SYS_SLEEP_CLOCK)) {
    // Use defaultBoardIndex if valid, otherwise fallback to first available
    // board
    int targetSlot = -1;

    if (config.defaultBoardIndex >= 0 &&
        config.defaultBoardIndex < MAX_BOARDS) {
      if (getDisplayBoard(config.defaultBoardIndex) != nullptr) {
        targetSlot = config.defaultBoardIndex;
      }
    }

    // If default invalid or not found, find first available
    if (targetSlot == -1) {
      for (int i = 0; i < MAX_BOARDS; i++) {
        if (getDisplayBoard(i) != nullptr) {
          targetSlot = i;
          break;
        }
      }
    }

    if (targetSlot != -1) {
      activeSlotIndex = targetSlot;
      showBoard(getDisplayBoard(activeSlotIndex),
                "Config Reload: Slot Re-enabled or Swapped");
    } else {
      activeSlotIndex = 0; // fallback if no boards were populated
    }
  } else {
    // It was sleeping, restore the sleep screen
    showBoard(previousBoard, "Config Reload: Restoring Sleep Screen");
  }
}

/**
 * @brief Unconditionally resumes the primary display carousel context.
 *        This effectively wakes up the display from sleep mode and transitions
 *        away from system screens (Splash, Loading, etc).
 */
void DisplayManager::resumeDisplays() {
  LOG_INFO("DISPLAY",
           "🔘 [DISPLAY] System ready/awake. Synchronizing with scheduler...");

  // Consult the scheduler for the starting board(s)
  std::vector<int> allowed = context->getSchedulerManager().getActiveBoards();
  int startIdx = activeSlotIndex; // Fallback

  if (!allowed.empty()) {
    startIdx = allowed[0];
    LOG_INFO("DISPLAY",
             String("🔘 [DISPLAY] Scheduler returned active board index: ") +
                 String(startIdx));
  } else {
    LOG_INFO("DISPLAY",
             String("🔘 [DISPLAY] No active schedule. Using default index: ") +
                 String(startIdx));
  }

  activeSlotIndex = startIdx;
  showBoard(getDisplayBoard(activeSlotIndex), "Context Resume: Scheduler Sync");
  u8g2.setContrast(brightness);
}
