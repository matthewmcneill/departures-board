/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/boards/systemBoard/sleepingBoard.cpp
 * Description: Implementation of the sleep mode screensaver board.
 */

#include "sleepingBoard.hpp"
#include <fonts/fonts.hpp>
#include "../../widgets/drawingPrimitives.hpp"
#include <timeManager.hpp>

SleepingBoard::SleepingBoard() 
    : showClock(true), dimmedBrightness(DIMMED_BRIGHTNESS), 
      bounceX(0), bounceY(0), lastBounce(0) {
}

void SleepingBoard::onActivate() {
    // Optimization: Reset bounce position on entry
    lastBounce = millis();
}

void SleepingBoard::onDeactivate() {
    // No specific cleanup needed
}

/**
 * @brief Updates the state of the sleeping board.
 * 
 * This method is called periodically to update the board's internal state,
 * such as shifting the clock position to prevent OLED burn-in.
 * 
 * @param ms The current system milliseconds.
 */
void SleepingBoard::tick(uint32_t ms) {
    // Shift position every 60 seconds to prevent OLED burn-in
    if (ms - lastBounce > 60000) {
        bounceX = random(20);
        bounceY = random(10);
        lastBounce = ms;
    }
}

void SleepingBoard::render(U8G2& display) {
    if (!showClock) return;

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return;

    char timeStr[6];
    char dateStr[20];
    sprintf(timeStr, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
    strftime(dateStr, sizeof(dateStr), "%d %b %Y", &timeinfo);

    display.setFont(NatRailClockLarge9);
    display.drawStr(60 + bounceX, 32 + bounceY, timeStr);
    display.setFont(NatRailSmall9);
    display.drawStr(60 + bounceX, 48 + bounceY, dateStr);
}
