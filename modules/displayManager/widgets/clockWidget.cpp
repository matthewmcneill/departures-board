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
 * - clockWidget: [Class implementation]
 *   - setFont: Update the primary typography.
 *   - setBlink: Toggle the 1Hz colon animation.
 *   - tick: Logic update for blink timing.
 *   - render: Primary drawing method.
 */

#include <fonts/fonts.hpp>
#include "clockWidget.hpp"
#include "drawingPrimitives.hpp"
#include <timeManager.hpp>
#include <time.h>

/**
 * @brief Initialize the clock with its time manager and default layout.
 * @param _timeMgr Pointer to the TimeManager dependency.
 * @param _x X coordinate.
 * @param _y Y coordinate.
 * @param _w Width (-1 for auto).
 * @param _h Height (-1 for auto).
 * @param _font Optional font override.
 */
clockWidget::clockWidget(TimeManager* _timeMgr, int _x, int _y, int _w, int _h, const uint8_t* _font)
    : iGfxWidget(_x, _y, _w, _h), showColon(true), oldColon(true), blinkEnabled(true), lastBlinkMs(0), lastMinute(-1), lastSecond(-1), format(ClockFormat::HH_MM), font(_font), secondaryFont(nullptr), timeMgr(_timeMgr) {
    if (font == nullptr) font = UndergroundClock8;
}

/**
 * @brief Update the font used for the time string.
 * @param newFont Pointer to the font array.
 */
void clockWidget::setFont(const uint8_t* newFont) {
    font = newFont;
}

void clockWidget::setSecondaryFont(const uint8_t* newFont) {
    secondaryFont = newFont;
}

void clockWidget::setFormat(ClockFormat newFormat) {
    format = newFormat;
}

/**
 * @brief Logic pulse for the blinking animation logic.
 * @param currentMillis System uptime in milliseconds.
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
    bool secondChanged = false;
    if (format == ClockFormat::HH_MM_SS) {
        secondChanged = (timeMgr->getCurrentTime().tm_sec != lastSecond);
    }

    if (oldColon != showColon || minuteChanged || secondChanged) {
        oldColon = showColon; // Fix: Update state to track changes
        lastMinute = timeMgr->getCurrentTime().tm_min;
        lastSecond = timeMgr->getCurrentTime().tm_sec;
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
    lastSecond = timeinfo.tm_sec;

    char hourStr[3];
    char minStr[3];
    char secStr[3];
    strftime(hourStr, sizeof(hourStr), "%H", &timeinfo);
    strftime(minStr, sizeof(minStr), "%M", &timeinfo);
    strftime(secStr, sizeof(secStr), "%S", &timeinfo);

    int hourW = display.getStrWidth(hourStr);
    int minW = display.getStrWidth(minStr);
    int colonW = display.getStrWidth(":");

    int maxHourW = display.getStrWidth("00");
    int maxMinW = display.getStrWidth("00");

    int secW = 0;
    int secColonW = 0;
    int maxSecW = 0;
    int maxSecColonW = 0;
    
    if (format == ClockFormat::HH_MM_SS) {
        if (secondaryFont) {
            display.setFont(secondaryFont);
        }
        secW = display.getStrWidth(secStr);
        maxSecW = display.getStrWidth("00");
        secColonW = display.getStrWidth(":");
        maxSecColonW = display.getStrWidth(":");
        display.setFont(font); // restore main font
    }

    // Calculate actual total footprint of the components
    int actualW = maxHourW + colonW + maxMinW + 2;
    if (format == ClockFormat::HH_MM_SS) {
        actualW += maxSecColonW + maxSecW + 2;
    }
    
    int startX = x;
    if (alignment == 1) { // CENTER
        startX = x + (renderW - actualW) / 2;
    } else if (alignment == 2) { // RIGHT
        startX = x + renderW - actualW - 1; // subtract 1 to prevent generic U8G2 string right-edge overhang clipping
    }

    // We use setFontPosTop globally in v3.0, so to vertically center text in renderH:
    // We calculate the remaining physical height after the font pixel height is subtracted, 
    // and split it into top and bottom padding.
    int fontHeight = display.getAscent() - display.getDescent();
    int drawY = y + (renderH - fontHeight) / 2;

    int secDrawY = drawY;
    if (format == ClockFormat::HH_MM_SS && secondaryFont) {
        display.setFont(secondaryFont);
        int secFontHeight = display.getAscent() - display.getDescent();
        // Align the bottoms of the two text boundaries (which natively flushes the numerical baselines)
        secDrawY = drawY + fontHeight - secFontHeight;
        display.setFont(font);
    }

    // Clearing the area prevents trailing artifacts
    blankArea(display, x, y, renderW, renderH);
    
    // Draw hours right-aligned within its max bounding box
    int hOff = maxHourW - hourW;
    display.drawStr(startX + hOff, drawY, hourStr);
    
    int currentX = startX + maxHourW + 1;
    
    // Draw colon if enabled
    if (showColon) {
        display.drawStr(currentX, drawY, ":");
    }
    currentX += colonW + 1;
    
    // Draw minutes right-aligned
    int mOff = maxMinW - minW;
    display.drawStr(currentX + mOff, drawY, minStr);
    currentX += maxMinW + 1;

    // Draw seconds
    if (format == ClockFormat::HH_MM_SS) {
        // The second colon separating minutes and seconds is standard continuous (non-blinking)
        if (secondaryFont) display.setFont(secondaryFont);
        
        int sColOff = (maxSecColonW - secColonW) / 2;
        display.drawStr(currentX + sColOff, secDrawY, ":");
        
        currentX += maxSecColonW + 1;
        
        int sOff = maxSecW - secW;
        display.drawStr(currentX + sOff, secDrawY, secStr);
        if (secondaryFont) display.setFont(font);
    }
}
