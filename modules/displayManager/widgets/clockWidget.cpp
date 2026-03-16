/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/gfxUtilities/src/clockWidget.cpp
 * Description: Implementation of the clockWidget with blinking colon logic.
 */

#include "clockWidget.hpp"
#include "drawingPrimitives.hpp"
#include <time.h>

extern struct tm timeinfo;

clockWidget::clockWidget(int _x, int _y, int _w, int _h, const uint8_t* _font)
    : iGfxWidget(_x, _y, _w, _h), showColon(true), blinkEnabled(true), lastBlinkMs(0), lastMinute(-1), font(_font) {
    if (font == nullptr) font = UndergroundClock8;
}

void clockWidget::setFont(const uint8_t* newFont) {
    font = newFont;
}

void clockWidget::tick(uint32_t currentMillis) {
    if (!isVisible || !blinkEnabled) return;

    if (currentMillis - lastBlinkMs >= 500) {
        showColon = !showColon;
        lastBlinkMs = currentMillis;
    }
}

void clockWidget::renderAnimationUpdate(U8G2& display, uint32_t currentMillis) {
    if (!isVisible) return;
    
    bool oldColon = showColon;
    tick(currentMillis);
    
    getLocalTime(&timeinfo);
    bool minuteChanged = (timeinfo.tm_min != lastMinute);

    if (oldColon != showColon || minuteChanged) {
        lastMinute = timeinfo.tm_min;
        render(display);
        int renderW = (width > 0) ? width : 56;
        int renderH = (height > 0) ? height : 14;
        display.updateDisplayArea(x / 8, y / 8, (renderW + 7) / 8, (renderH + 7) / 8);
    }
}

void clockWidget::render(U8G2& display) {
    if (!isVisible) return;

    int renderW = (width > 0) ? width : 56;
    int renderH = (height > 0) ? height : 14;

    display.setFont(font);

    char timeStr[9];
    if (showColon) {
        strftime(timeStr, sizeof(timeStr), "%H:%M", &timeinfo);
    } else {
        strftime(timeStr, sizeof(timeStr), "%H %M", &timeinfo);
    }

    int w = display.getStrWidth(timeStr);
    int startX = x + (renderW - w) / 2;
    // Clearing the area is usually handled by the board, but for a clock we might want to be precise
    blankArea(display, x, y, renderW, renderH);
    display.drawStr(startX, y + renderH - 4, timeStr);
}
