/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/displayManager/displayManager.cpp
 * Description: Implementation of the DisplayManager singleton orchestrator.
 *              Manages hardware display state, power management, and board rendering.
 *
 * Exported Functions/Classes:
 * - DisplayManager::DisplayManager: Constructor initializing default state.
 * - DisplayManager::begin: Initialize hardware display driver.
 * - DisplayManager::tick: Executive rendering and state management loop.
 * - DisplayManager::render: Forces a immediate synchronized display refresh.
 * - DisplayManager::showBoard: Switch to a new board and optionally block for an animation duration.
 * - DisplayManager::applyConfig: Provisions system state from a central config object.
 */

#include "displayManager.hpp"
#include "widgets/drawingPrimitives.hpp"
#include <timeManager.hpp>
#include <Logger.hpp>
#include <weatherClient.h>
#include <rssClient.h>
#include <otaUpdater.hpp>
#include <boards/systemBoard/sleepingBoard.hpp>
#include "configManager.hpp"
#include <appContext.hpp>

// DisplayManager singleton will be owned by appContext

bool DisplayManager::getFlipScreen() const { return flipScreen; }
bool DisplayManager::getSleepEnabled() const { return sleepEnabled; }
void DisplayManager::setSleepEnabled(bool enabled) { sleepEnabled = enabled; }
byte DisplayManager::getSleepStarts() const { return sleepStarts; }
void DisplayManager::setSleepStarts(byte starts) { sleepStarts = starts; }
byte DisplayManager::getSleepEnds() const { return sleepEnds; }
void DisplayManager::setSleepEnds(byte ends) { sleepEnds = ends; }
bool DisplayManager::getForcedSleep() const { return forcedSleep; }
void DisplayManager::setForcedSleep(bool active) { forcedSleep = active; }

// --- Singleton Instance ---
// This will be removed in favor of appContext ownership.
// DisplayManager displayManager; // Global system display orchestrator

// --- Lifecycle Methods ---

/**
 * @brief Default constructor. Pre-configures the u8g2 hardware driver with SPI pins.
 */
DisplayManager::DisplayManager() 
    : u8g2(U8G2_R0, /* cs=*/ DISPLAY_CS_PIN, /* dc=*/ DISPLAY_DC_PIN, /* reset=*/ U8X8_PIN_NONE),
      activeSlotIndex(0), brightness(20), flipScreen(false), 
      sleepEnabled(false), sleepStarts(23), sleepEnds(8), 
      currentBoard(nullptr), lastActivity(0), forcedSleep(false),
      wifiWarning(0, 56), otaUpdateAvailable(false) {
}

/**
 * @brief Initialize the hardware display and apply initial buffer settings.
 * @param contextPtr Pointer to the parent application context.
 */
void DisplayManager::begin(appContext* contextPtr) {
    context = contextPtr;
    
    // Initialize system boards with context
    splashBoard.init(context);
    loadingBoard.init(context);
    wizardBoard.init(context);
    helpBoard.init(context);
    messageBoard.init(context);
    firmwareUpdateBoard.init(context);
    sleepingBoard.init(context);

    u8g2.begin();
    u8g2.setDrawColor(1);
    u8g2.setFontMode(1);
    u8g2.setFontRefHeightAll();
    u8g2.setFontPosTop();
    u8g2.setFont(NatRailTall12);
    
    lastActivity = millis();
}

/**
 * @brief Top-level execution tick. Handles sleep state transitions,
 *        board logic updates, and triggers rendering.
 * @param currentMillis Current system uptime.
 */
void DisplayManager::tick(unsigned long currentMillis) {
    // --- Step 1: Handle Sleep Transitions ---
    bool shouldSnooze = isSnoozing();
    iDisplayBoard* sleepBoard = getSystemBoard(SystemBoardId::SYS_SLEEP_CLOCK);

    if (shouldSnooze && currentBoard != sleepBoard) {
        // Transition TO Sleep (Dimming and switching to clock)
        showBoard(sleepBoard);
        u8g2.setContrast(((SleepingBoard*)sleepBoard)->getDimmedBrightness());
    } else if (!shouldSnooze && currentBoard == sleepBoard) {
        // Transition FROM Sleep (Restoring brightness and active carousel board)
        showBoard(getDisplayBoard(activeSlotIndex));
        u8g2.setContrast(brightness);
        u8g2.clearDisplay(); // Ensure a clean wake-up surface
    }

    // --- Step 2: Update Logic and Render ---
    if (currentBoard != nullptr) currentBoard->tick(currentMillis);
    
    wifiWarning.tick(currentMillis);
    
    render();
}

/**
 * @brief Perform a full synchronized render transaction.
 * @note Manages clearBuffer(), rendering the active/override board, and sendBuffer().
 */
void DisplayManager::render() {
    u8g2.clearBuffer();

    // Render the unified current target
    if (currentBoard != nullptr) {
        currentBoard->render(u8g2);
    }

    // Render Global Overlays
    wifiWarning.render(u8g2);
    
    if (otaUpdateAvailable) {
        u8g2.setFont(NatRailSmall9);
        u8g2.drawStr(0, 50, "\xAB");
    }

    u8g2.sendBuffer();
}

iDisplayBoard* DisplayManager::getCurrentBoard() { return currentBoard; }

/**
 * @brief Switch to a new board and optionally block for an animation duration.
 * @param board Pointer to the board implementation to render.
 * @param durationMs Optional duration in ms to block and animate.
 */
void DisplayManager::showBoard(iDisplayBoard* board, uint32_t durationMs) {
    if (currentBoard == board) return;

    if (currentBoard != nullptr) {
        currentBoard->onDeactivate();
    }

    currentBoard = board;
    
    if (currentBoard != nullptr) {
        currentBoard->onActivate();
        LOG_INFO("DISPLAY", "Board activated.");
    }
    
    // Initial full render
    render();

    if (durationMs > 0) {
        unsigned long end = millis() + durationMs;
        // Animation loop
        while (millis() < end) {
            yieldAnimationUpdate();
            delay(1); // Feed watchdog and yield to RTOS
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
iDisplayBoard* DisplayManager::getSystemBoard(SystemBoardId id) {
    switch (id) {
        case SystemBoardId::SYS_BOOT_SPLASH:
            return &splashBoard;
        case SystemBoardId::SYS_BOOT_LOADING:
            loadingBoard.setHeading("Departures Board");
            loadingBoard.setBuildTime(String(__DATE__ " " __TIME__).c_str());
            return &loadingBoard;
        case SystemBoardId::SYS_WIFI_WIZARD:
            wizardBoard.setWizardIp(WiFi.softAPIP());
            return &wizardBoard;
        case SystemBoardId::SYS_HELP_KEYS: {
            const char* lines[] = {
                "Check National Rail", "Website -> My Account ->", "OpenLDBWS -> get token.", "TfL: API Portal -> keys.", "Connect to config AP."
            };
            helpBoard.setHelpContent("First Boot Keys Help", lines, 5);
            return &helpBoard;
        }
        case SystemBoardId::SYS_HELP_CRS: {
            const char* lines[] = {
                "National Rail specific.", "This is the 3 letter", "code for a station.", "Eg EUS=Euston VIC=Victoria", "Connect to config AP."
            };
            helpBoard.setHelpContent("CRS Help", lines, 5);
            return &helpBoard;
        }
        case SystemBoardId::SYS_ERROR_NO_DATA:
            messageBoard.setContent("** COMMS ERROR **", "NO DEPARTURE DATA", "UNABLE TO GET DATA FOR", "STATION.");
            return &messageBoard;
        case SystemBoardId::SYS_ERROR_WSDL:
            messageBoard.setContent("** WSDL ERROR **", "FAILED TO INIT API", "CANNOT CONTACT NATIONAL", "RAIL SERVERS.");
            return &messageBoard;
        case SystemBoardId::SYS_ERROR_TOKEN:
            messageBoard.setContent("** AUTHORISATION **", "INVALID API KEY", "CHECK YOUR TOKEN AND", "TFL CREDENTIALS VIA", "THE WEB SETUP WIZARD.");
            return &messageBoard;
        case SystemBoardId::SYS_ERROR_CRS:
            messageBoard.setContent("** STATION ERROR **", "INVALID CRS/NAPTAN", "STATION CODE NOT", "RECOGNISED BY API.");
            return &messageBoard;
        case SystemBoardId::SYS_FIRMWARE_UPDATE:
            return &firmwareUpdateBoard;
        case SystemBoardId::SYS_SLEEP_CLOCK:
            return &sleepingBoard;
        default:
            return nullptr;
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
MessagePool* DisplayManager::getGlobalMessagePool() { 
    return (context != nullptr) ? &context->getGlobalMessagePool() : nullptr;
}

/**
 * @brief Checks if the system is currently in a sleep state.
 * @return True if the current board is the sleep clock.
 */
bool DisplayManager::getIsSleeping() const { 
    return (currentBoard != nullptr && currentBoard == const_cast<DisplayManager*>(this)->getSystemBoard(SystemBoardId::SYS_SLEEP_CLOCK)); 
}

/**
 * @brief Checks if the display should be snoozing based on manual override or schedule.
 * @return True if system should transition to sleep.
 */
bool DisplayManager::isSnoozing() {
    if (forcedSleep) return true;
    if (!sleepEnabled) return false;
    
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)) return false; // Time sync required for schedule
    
    byte myHour = timeinfo.tm_hour;
    if (sleepStarts > sleepEnds) {
        // Range spans midnight
        if ((myHour >= sleepStarts) || (myHour < sleepEnds)) return true; else return false;
    } else {
        // Standard range within one day
        if ((myHour >= sleepStarts) && (myHour < sleepEnds)) return true; else return false;
    }
}

// --- Board Management ---

/**
 * @brief Retrieve a pointer to a board in the carousel pool.
 * @param slotIndex Index (0 to MAX_BOARDS-1).
 * @return iDisplayBoard* Pointer to active implementation, or nullptr if empty.
 */
iDisplayBoard* DisplayManager::getDisplayBoard(int slotIndex) {
    if (slotIndex < 0 || slotIndex >= MAX_BOARDS) return nullptr;
    
    if (std::holds_alternative<NationalRailBoard>(slots[slotIndex])) {
        return &std::get<NationalRailBoard>(slots[slotIndex]);
    } else if (std::holds_alternative<TfLBoard>(slots[slotIndex])) {
        return &std::get<TfLBoard>(slots[slotIndex]);
    } else if (std::holds_alternative<BusBoard>(slots[slotIndex])) {
        return &std::get<BusBoard>(slots[slotIndex]);
    }
    return nullptr; 
}

BoardType DisplayManager::getActiveBoardType() {
    if (std::holds_alternative<NationalRailBoard>(slots[activeSlotIndex])) return BoardType::NR_BOARD;
    if (std::holds_alternative<TfLBoard>(slots[activeSlotIndex])) return BoardType::TFL_BOARD;
    if (std::holds_alternative<BusBoard>(slots[activeSlotIndex])) return BoardType::BUS_BOARD;
    return BoardType::NR_BOARD;
}

/**
 * @brief Allocates a specific Board type into a designated Carousel slot.
 * @param slotIndex Index (0 to MAX_BOARDS-1).
 * @param type Board implementation type to instantiate.
 */
void DisplayManager::setBoardType(int slotIndex, BoardType type) {
    if (slotIndex < 0 || slotIndex >= MAX_BOARDS) return;
    switch (type) {
        case BoardType::NR_BOARD: slots[slotIndex].emplace<NationalRailBoard>(context); break;
        case BoardType::TFL_BOARD: slots[slotIndex].emplace<TfLBoard>(context); break;
        case BoardType::BUS_BOARD: slots[slotIndex].emplace<BusBoard>(context); break;
    }
}

void DisplayManager::clearSlot(int slotIndex) {
    if (slotIndex >= 0 && slotIndex < MAX_BOARDS) slots[slotIndex] = std::monostate{};
}

BoardVariant* DisplayManager::getSlot(int slotIndex) {
    if (slotIndex >= 0 && slotIndex < MAX_BOARDS) return &slots[slotIndex];
    return nullptr;
}


/**
 * @brief Free all carousel slots and reset to uninitialized state.
 */
void DisplayManager::clearSlots() {
    for (int i = 0; i < MAX_BOARDS; i++) {
        slots[i].emplace<std::monostate>();
    }
}

/**
 * @brief Add a board of the specified type to the first available slot.
 * @param type Board implementation type.
 * @return iDisplayBoard* Pointer to the new board, or nullptr if full.
 */
iDisplayBoard* DisplayManager::addBoard(BoardType type) {
    for (int i = 0; i < MAX_BOARDS; i++) {
        if (std::holds_alternative<std::monostate>(slots[i])) {
            setBoardType(i, type);
            return getDisplayBoard(i);
        }
    }
    return nullptr;
}

/**
 * @brief Rotates the carousel index to the next initialized board.
 * @return The new active slot index.
 */
int DisplayManager::cycleNext() {
    int start = activeSlotIndex;
    do {
        activeSlotIndex = (activeSlotIndex + 1) % MAX_BOARDS;
        iDisplayBoard* board = getDisplayBoard(activeSlotIndex);
        if (board != nullptr) {
            showBoard(board);
            return activeSlotIndex;
        }
    } while (activeSlotIndex != start);
    return activeSlotIndex;
}

// --- Rendering Helpers ---

/**
 * @brief Logic scroller that provides sub-ms timing to boards for scrolling animations.
 *        Capped at ~60fps (16ms per update).
 */
void DisplayManager::yieldAnimationUpdate() {
    static uint32_t lastAnim = 0;
    uint32_t now = millis();
    
    if (now - lastAnim >= 16) { 
        lastAnim = now;
        if (currentBoard) currentBoard->renderAnimationUpdate(u8g2, now);
        
        // Directly update hardware buffer with animation state of overlay
        wifiWarning.renderAnimationUpdate(u8g2, now);
        if (otaUpdateAvailable) {
            u8g2.setFont(NatRailSmall9);
            u8g2.drawStr(0, 50, "\xAB");
            u8g2.updateDisplayArea(0, 6, 1, 2);
        }
    }
}

/**
 * @brief Provisions the hardware display and carousel based on a central config.
 * @param config Reference to the source configuration struct.
 */
void DisplayManager::applyConfig(const Config& config) {
    LOG_INFO("DISPLAY", "Applying configuration to DisplayManager...");
    
    // --- Step 1: Apply Global Hardware and Power settings ---
    setBrightness(config.brightness);
    setFlipScreen(config.flipScreen);
    setSleepEnabled(config.sleepEnabled);
    setSleepStarts((byte)config.sleepStarts);
    setSleepEnds((byte)config.sleepEnds);

    // Sync extra attributes to specialized boards
    SleepingBoard* sb = (SleepingBoard*)getSystemBoard(SystemBoardId::SYS_SLEEP_CLOCK);
    sb->setShowClock(config.showClockInSleep);

    // --- Step 2: Clear and Re-provision Carousel Slots ---
    clearSlots();

    for (int i = 0; i < config.boardCount; i++) {
        const BoardConfig& bc = config.boards[i];
        LOG_INFO("DISPLAY", String("Provisioning Board ") + i + " (Type: " + (int)bc.mode + ")");

        switch (bc.mode) {
            case MODE_RAIL: {
                setBoardType(i, BoardType::NR_BOARD);
                NationalRailBoard* rb = (NationalRailBoard*)getDisplayBoard(i);
                if (rb) {
                    rb->setNrToken(config.nrToken);
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
                TfLBoard* tb = (TfLBoard*)getDisplayBoard(i);
                if (tb) {
                    tb->setTflAppkey(config.tflAppkey);
                    tb->setTubeId(bc.id);
                    tb->setTubeName(bc.name);
                }
                break;
            }
            case MODE_BUS: {
                setBoardType(i, BoardType::BUS_BOARD);
                BusBoard* bb = (BusBoard*)getDisplayBoard(i);
                if (bb) {
                    bb->setBusAtco(bc.id);
                    bb->setBusName(bc.name);
                    bb->setBusLat(bc.lat);
                    bb->setBusLon(bc.lon);
                    bb->setBusFilter(bc.filter);
                }
                break;
            }
        }

        iDisplayBoard* dispBoard = getDisplayBoard(i);
        if (dispBoard) dispBoard->configure(bc);
    }

    // --- Step 3: Finalize Initial View ---
    if (currentBoard != getSystemBoard(SystemBoardId::SYS_SLEEP_CLOCK)) {
        // Use defaultBoardIndex if valid, otherwise fallback to first available board
        int targetSlot = -1;
        
        if (config.defaultBoardIndex >= 0 && config.defaultBoardIndex < MAX_BOARDS) {
           if (getDisplayBoard(config.defaultBoardIndex) != nullptr) {
               targetSlot = config.defaultBoardIndex;
           }
        }

        // If default invalid or not found, find first available
        if (targetSlot == -1) {
            for(int i=0; i<MAX_BOARDS; i++) {
               if(getDisplayBoard(i) != nullptr) {
                  targetSlot = i;
                  break;
               }
            }
        }

        if (targetSlot != -1) {
           activeSlotIndex = targetSlot;
           showBoard(getDisplayBoard(activeSlotIndex));
        } else {
           activeSlotIndex = 0; // fallback if no boards were populated
        }
    }
}

/**
 * @brief Resets the display manager state. Wakes up the screen if it was snoozing.
 */
void DisplayManager::resetState() {
    // If we were sleeping, wake up immediately
    if (getIsSleeping()) {
        showBoard(getDisplayBoard(activeSlotIndex));
        u8g2.setContrast(brightness);
    }
}
