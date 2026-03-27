/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/boards/systemBoard/sleepingBoard.hpp
 * Description: Low-power screensaver board. Implements a bouncing large-format 
 *              clock designed to prevent OLED burn-in/ghosting while the board 
 *              is in a scheduled sleep state.
 *
 * Exported Functions/Classes:
 * - SleepingBoard: System board for inactivity periods.
 *   - setShowClock(): Toggle time visibility.
 *   - setDimmedBrightness(): Configure display contrast for night mode.
 *   - onActivate() / onDeactivate(): Lifecycle hooks for display transitions.
 *   - tick(): Logic update for burn-in protection shifts.
 *   - render(): Primary drawing method for the large clock.
 */

#pragma once

class appContext;

#include "../interfaces/iDisplayBoard.hpp"
#include <U8g2lib.h>
#include <Arduino.h>

/**
 * @brief Display board for sleep mode, featuring a bouncing clock for burn-in prevention.
 */
class SleepingBoard : public iDisplayBoard {
private:
    appContext* context;
    bool showClock;           ///< Whether to render the time and date
    int dimmedBrightness;     ///< Contrast level to apply when active
    
    int bounceX;              ///< Internal horizontal offset for burn-in protection
    int bounceY;              ///< Internal vertical offset for burn-in protection
    unsigned long lastBounce; ///< Timestamp of the last position shift
    WeatherStatus weatherStatus;
    bool oledOff;             ///< Whether to power down the OLED when active

public:
    SleepingBoard(appContext* contextPtr = nullptr);
    const char* getBoardName() const override { return "SYS: Sleep Clock"; }

    /**
     * @brief Handle the bouncing logic for screensaver positioning.
     * @param currentMillis Current system time.
     */
    void onActivate() override;
    void onDeactivate() override;
    void configure(const struct BoardConfig& config) override;
    void tick(uint32_t ms) override;

    /**
     * @brief Render the sleep clock and date.
     * @param display Reference to the hardware display driver.
     */
    void render(U8G2& display) override;

    // Setters / Getters
    void setShowClock(bool show) { showClock = show; }
    bool getShowClock() const { return showClock; }

    void setDimmedBrightness(int level) { dimmedBrightness = level; }
    int getDimmedBrightness() const { return dimmedBrightness; }

    void setOledOff(bool off) { oledOff = off; }
    bool getOledOff() const { return oledOff; }

    /**
     * @brief Inject the application context.
     * @param contextPtr Pointer to context.
     */
    void init(appContext* contextPtr) { context = contextPtr; }

    // Minimal interface implementations
    /** @return Always 0 for system boards. */
    int updateData() override { return 0; }

    /** @brief No-op animation tick. */
    void renderAnimationUpdate(U8G2& display, uint32_t currentMillis) override {}

    /** @return Empty string for system boards. */
    const char* getLastErrorMsg() override { return ""; }

    /**
     * @brief Access the shared weather state (unused on sleep).
     * @return WeatherStatus reference.
     */
    WeatherStatus& getWeatherStatus() override { return weatherStatus; }
};
