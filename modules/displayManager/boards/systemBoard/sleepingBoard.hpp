/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/boards/systemBoard/sleepingBoard.hpp
 * Description: Specialized display board for sleep mode/screensaver.
 *              Encapsulates burn-in protection and clock rendering.
 *
 * Exported Functions/Classes:
 * - SleepingBoard: iDisplayBoard implementation for the sleep clock.
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

protected:
    SleepingBoard();
    friend class DisplayManager;

public:

    /**
     * @brief Handle the bouncing logic for screensaver positioning.
     * @param currentMillis Current system time.
     */
    void onActivate() override;
    void onDeactivate() override;
    void configure(const struct BoardConfig& config) override { (void)config; }
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

    void init(appContext* contextPtr) { context = contextPtr; }

    // Minimal interface implementations
    int updateData() override { return 0; }
    void renderAnimationUpdate(U8G2& display, uint32_t currentMillis) override {}
    const char* getLastErrorMsg() override { return ""; }
    WeatherStatus& getWeatherStatus() override { return weatherStatus; }
};
