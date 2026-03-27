#include <fonts/fonts.hpp>
/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/widgets/clockWidget.cpp
 * Description: Implementation of the real-time clock drawing logic.
 *
 * Exported Functions/Classes:
 * - clockWidget: Graphical widget for real-time display.
 *   - setFont(): Update the typography.
 *   - setBlink(): Toggle the 1Hz colon animation.
 *   - tick(): Logic update for blink timing.
 *   - render(): Primary drawing method.
 *   - renderAnimationUpdate(): Targeted redraw for the colon/minutes.
 */

#include "clockWidget.hpp"
#include "drawingPrimitives.hpp"
#include <timeManager.hpp>
#include <time.h>

/**
 * @brief Initialize the clock with its time manager and layout.
 */
clockWidget::clockWidget(TimeManager* _timeMgr, int _x, int _y, int _w, int _h, const uint8_t* _font)
    : iGfxWidget(_x, _y, _w, _h), showColon(true), oldColon(true), blinkEnabled(true), lastBlinkMs(0), lastMinute(-1), font(_font), timeMgr(_timeMgr) {
    if (font == nullptr) font = UndergroundClock8;
}

/**
 * @brief Update the font used for the time string.
 * @param newFont Pointer to the font array.
 */
void clockWidget::setFont(const uint8_t* newFont) {
    font = newFont;
}

/**
 * @brief Handle 1Hz pulse for the colon blink.
 * @param currentMillis Milliseconds since boot.
 */
void clockWidget::tick(uint32_t currentMillis) {
    if (!isVisible || !blinkEnabled) return;

    if (currentMillis - lastBlinkMs >= 500) {
        showColon = !showColon;
        lastBlinkMs = currentMillis;
    }
}

/**
 * @brief Partial redraw for the blinking colon or minute change.
 * @param display U8g2 reference.
 * @param currentMillis Milliseconds since boot.
 */
void clockWidget::renderAnimationUpdate(U8G2& display, uint32_t currentMillis) {
    if (!isVisible) return;
    
    if (!timeMgr) return;
    
    timeMgr->updateCurrentTime();
    bool minuteChanged = (timeMgr->getCurrentTime().tm_min != lastMinute);

    if (oldColon != showColon || minuteChanged) {
        oldColon = showColon; // Fix: Update state to track changes
        lastMinute = timeMgr->getCurrentTime().tm_min;
        render(display);
        int renderW = (width > 0) ? width : 56;
        int renderH = (height > 0) ? height : 14;
        display.updateDisplayArea(x / 8, y / 8, (renderW + 7) / 8, (renderH + 7) / 8);
    }
}

/**
 * @brief Full frame render for the time string.
 * @param display U8g2 reference.
 */
void clockWidget::render(U8G2& display) {
    if (!isVisible) return;

    int renderW = (width > 0) ? width : 56;
    int renderH = (height > 0) ? height : 14;

    // Save global display state (font selection, colors, clippers) before modifying
    U8g2StateSaver stateSaver(display);

    display.setFont(font);

    if (!timeMgr) return;

    // Fix: Always update current time during a full render pass to keep clock fresh
    timeMgr->updateCurrentTime();
    const struct tm& timeinfo = timeMgr->getCurrentTime();
    lastMinute = timeinfo.tm_min; // Keep in sync with animation updates

    char hourStr[3];
    char minStr[3];
    strftime(hourStr, sizeof(hourStr), "%H", &timeinfo);
    strftime(minStr, sizeof(minStr), "%M", &timeinfo);

    int hourW = display.getStrWidth(hourStr);
    int minW = display.getStrWidth(minStr);
    int colonW = display.getStrWidth(":");

    // Calculate total width using the colon's width as the fixed gap, plus 1px padding on each side
    int totalW = hourW + colonW + minW + 2;
    int startX = x + (renderW - totalW) / 2;

    // We use setFontPosTop globally in v3.0, so to vertically center text in renderH:
    // We calculate the remaining physical height after the font pixel height is subtracted, 
    // and split it into top and bottom padding.
    int fontHeight = display.getAscent() - display.getDescent();
    int drawY = y + (renderH - fontHeight) / 2;

    // Clearing the area prevents trailing artifacts
    blankArea(display, x, y, renderW, renderH);
    
    // Abstract the vertical geometries directly using U8g2 raw calls to avoid clipping pipeline overhead
    display.drawStr(startX, drawY, hourStr);
    
    // Draw colon if enabled
    if (showColon) {
        display.drawStr(startX + hourW + 1, drawY, ":");
    }
    
    // Draw minutes
    display.drawStr(startX + hourW + colonW + 2, drawY, minStr);
}
