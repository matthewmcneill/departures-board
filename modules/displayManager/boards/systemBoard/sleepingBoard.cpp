/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/boards/systemBoard/sleepingBoard.cpp
 * Description: Implementation of the burn-in protection sleep clock.
 */

#include <appContext.hpp>
#define DIMMED_BRIGHTNESS 15
#include "sleepingBoard.hpp"
#include <fonts/fonts.hpp>
#include "../../widgets/drawingPrimitives.hpp"
#include <timeManager.hpp>
#include "../../displayManager.hpp"

/**
 * @brief Initialize the sleep board with burn-in protection enabled.
 */
SleepingBoard::SleepingBoard(appContext* contextPtr) 
    : context(contextPtr), showClock(true), dimmedBrightness(2), 
      bounceX(0), bounceY(0), lastBounce(0), oledOff(false) {
}

/**
 * @brief Lifecycle hook called when the sleep board becomes active.
 */
void SleepingBoard::onActivate() {
    lastBounce = millis();
    // Centrally managed by DisplayManager::showBoard() - setPowerSave(false) 
    // happens before this. We selectively turn it OFF here.
    if (oledOff && context) {
        context->getDisplayManager().setPowerSave(true);
    }
}

/**
 * @brief Lifecycle hook for board exit.
 */
void SleepingBoard::onDeactivate() {
    // No specific cleanup needed
}

/**
 * @brief Configure the sleeping board with specific settings.
 * @param config The configuration object for the board.
 */
void SleepingBoard::configure(const BoardConfig& config) {
    oledOff = config.oledOff;
}

/**
 * @brief Per-tick state update for burn-in protection origin shifting.
 * @param ms Current system time in milliseconds.
 */
void SleepingBoard::tick(uint32_t ms) {
    // --- burn-in protection ---
    // Shift the rendering origin every 60 seconds to ensure consistent sub-pixel aging.
    if (ms - lastBounce > 60000) {
        bounceX = random(20);
        bounceY = random(10);
        lastBounce = ms;
    }
}

/**
 * @brief Render the oversized bouncing clock.
 * @param display Reference to U8g2.
 */
void SleepingBoard::render(U8G2& display) {
    if (!showClock) return;

    // --- Step 1: Time Acquisition ---
    context->getTimeManager().updateCurrentTime();
    const struct tm& timeinfo = context->getTimeManager().getCurrentTime();

    char timeStr[6];
    char dateStr[20];
    sprintf(timeStr, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
    strftime(dateStr, sizeof(dateStr), "%d %b %Y", &timeinfo);

    // --- Step 2: Offset Rendering ---
    // Apply the burn-in bounce offsets to all screen elements.
    display.setFont(NatRailClockLarge9);
    display.drawStr(60 + bounceX, 32 + bounceY, timeStr);
    
    display.setFont(NatRailSmall9);
    display.drawStr(60 + bounceX, 48 + bounceY, dateStr);
}
