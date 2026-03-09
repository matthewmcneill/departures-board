/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/displayManager/displayManager.hpp
 * Description: Memory pool utilizing std::variant to statically allocate
 *              the memory required for up to MAX_BOARDS departures without 
 *              fragmenting the heap during run-time. Also manages global
 *              screen sleep states and screen dimming.
 *
 * Provides:
 * - DisplayManager: Singleton class managing screen state, sleep timers, and rendering.
 * - displayManager: Global instantiated object of DisplayManager.
 */

#pragma once

#include <variant>
#include <JsonListener.h>
#include <JsonStreamingParser.h>
#include "../boards/nationalRailBoard/include/nationalRailBoard.hpp"
#include "../boards/tflBoard/include/tflBoard.hpp"
#include "../boards/busBoard/include/busBoard.hpp"

// Forward declaration of the globally configured MAX_BOARDS
// defined in platformio.ini (Default to 3 if missing)
#ifndef MAX_BOARDS
#define MAX_BOARDS 3
#endif

// How often the screen is changed in sleep mode (ms)
#define SCREENSAVERINTERVAL 10000

enum class BoardType {
    NR_BOARD,
    TFL_BOARD,
    BUS_BOARD
};

/**
 * @brief Type-safe union overlay spanning the maximum memory footprint 
 *        of any hardware Board class implementation.
 */
using BoardVariant = std::variant<std::monostate, NationalRailBoard, TfLBoard, BusBoard>;

class DisplayManager {
private:
    BoardVariant slots[MAX_BOARDS];
    int activeSlotIndex;

    // Display configuration set by ConfigManager
    int brightness;
    bool flipScreen;
    bool sleepEnabled;
    byte sleepStarts;
    bool sleepEnds;

    // Display State
    bool sleepClock;
    bool isSleeping;
    bool forcedSleep;
    unsigned long timer;

    // Internal screensaver logic
    
    /**
     * @brief Render the screensaver with an obscured floating clock to prevent OLED burn-in.
     */
    void drawSleepingScreen();

public:
    int getBrightness() const;
    void setBrightness(int newBrightness);

    bool getFlipScreen() const;
    void setFlipScreen(bool newFlipScreen);

    bool getSleepEnabled() const;
    void setSleepEnabled(bool newSleepEnabled);

    byte getSleepStarts() const;
    void setSleepStarts(byte newSleepStarts);

    byte getSleepEnds() const;
    void setSleepEnds(byte newSleepEnds);

    bool getForcedSleep() const;
    void setForcedSleep(bool active);

    bool getSleepClock() const;
    void setSleepClock(bool active);

    bool getIsSleeping() const;

    bool isSnoozing();
    DisplayManager();

    // Main execution loop for rendering the active board or the screensaver
    void tick(unsigned long currentMillis);

    // Reset loop state (used during soft softResetBoard)
    void resetState();

    /**
     * @brief Get current hardware execution mode wrapped inside the active board.
     * @return BoardType enum indicating active board API type.
     */
    BoardType getActiveBoardType();

    /**
     * @brief Allocates a specific Board type into a designated Carousel slot.
     */
    void setBoardType(int slotIndex, BoardType type);

    /**
     * @brief Clears a particular slot back to uninitialized memory.
     */
    void clearSlot(int slotIndex);

    /**
     * @brief Obtain the specific Board object stored in a slot.
     */
    BoardVariant* getSlot(int slotIndex);

    /**
     * @brief Rotates the carousel to the next initialized board.
     */
    int cycleNext();

    /**
     * @brief Return the globally active IStation pointer for polymorphic drawing bounds.
     */
    IStation* getActiveBoard();
    
    /**
     * @brief Get the active array index currently rendered in the Carousel.
     * @return Zero-indexed integer showing board position.
     */
    int getActiveSlotIndex() const;
};

// Global instance exposed
extern DisplayManager displayManager;
