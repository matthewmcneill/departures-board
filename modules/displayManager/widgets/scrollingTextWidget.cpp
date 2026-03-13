/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/widgets/scrollingTextWidget.cpp
 * Description: Implementation of generic marquee scrolling logic.
 */

#include "scrollingTextWidget.hpp"
#include "drawingPrimitives.hpp"
#include <displayManager.hpp>

extern DisplayManager displayManager;

/**
 * @brief Construct a new scrolling text widget.
 */
scrollingTextWidget::scrollingTextWidget(int _x, int _y, int _w, int _h, const uint8_t* _font)
    : iGfxWidget(_x, _y, _w, _h), scrollX(0), messageWidth(0), lastScrollMs(0), 
      needsScroll(false), needsLayout(true), font(_font), scrollFinished(false) {
    if (font == nullptr) font = NatRailSmall9;
    currentText[0] = '\0';
}

/**
 * @brief Assigns new text to the widget, resetting alignment and math.
 */
void scrollingTextWidget::setText(const char* newText) {
    if (newText == nullptr) {
        currentText[0] = '\0';
        needsLayout = true;
        return;
    }
    
    // Ignore if identical to prevent layout thrash
    if (strcmp(currentText, newText) == 0) return;

    strncpy(currentText, newText, sizeof(currentText) - 1);
    currentText[sizeof(currentText) - 1] = '\0';
    
    needsLayout = true;
    scrollFinished = false;
}

/**
 * @brief Rewinds scroll to start position and sets delays.
 */
void scrollingTextWidget::resetScroll() {
    scrollX = 0;
    lastScrollMs = 0; // Trigger initialization delay down in tick()
    scrollFinished = false;
}

/**
 * @brief Main progression math for animation ticks.
 */
void scrollingTextWidget::tick(uint32_t currentMillis) {
    if (!isVisible || !needsScroll) return;

    // Initial load delay
    if (lastScrollMs == 0) {
        lastScrollMs = currentMillis + 3000; 
        return;
    }

    if (currentMillis > lastScrollMs) {
        scrollX++;
        int maxW = (width > 0) ? width : SCREEN_WIDTH;
        
        // Check if string has fully exited screen left
        if (scrollX > messageWidth + 30) {
            scrollFinished = true; // Signal derived classes
            scrollX = -maxW;       // Wrap completely around to right edge
        }
        
        lastScrollMs = currentMillis + 40; // ~25fps pacing
    }
}

/**
 * @brief Paints the widget entirely on-demand.
 */
void scrollingTextWidget::render(U8G2& display) {
    if (!isVisible || currentText[0] == '\0') return;

    if (needsLayout) {
        display.setFont(font);
        messageWidth = getStringWidth(display, currentText);
        int maxW = (width > 0) ? width : SCREEN_WIDTH;
        needsScroll = (messageWidth > maxW);
        resetScroll();
        needsLayout = false;
    }

    int renderW = (width > 0) ? width : SCREEN_WIDTH;
    int renderH = (height > 0) ? height : 12;

    blankArea(display, x, y, renderW, renderH);

    display.setFont(font);
    
    if (needsScroll) {
        display.setClipWindow(x, y, x + renderW, y + renderH);
        display.drawStr(x - scrollX, y + renderH - 2, currentText);
        display.setMaxClipWindow();
    } else {
        // If it fits, we just center it statically
        int w = display.getStrWidth(currentText);
        int start = x + (renderW - w) / 2;
        display.drawStr(start, y + renderH - 2, currentText);
        scrollFinished = true; // Automatically signal done since it never scrolls
    }
}

/**
 * @brief Partial refresh for high-speed non-blocking updates.
 */
void scrollingTextWidget::renderAnimationUpdate(U8G2& display, uint32_t currentMillis) {
    if (!isVisible || currentText[0] == '\0') return;

    if (needsLayout) {
        display.setFont(font);
        messageWidth = getStringWidth(display, currentText);
        int maxW = (width > 0) ? width : SCREEN_WIDTH;
        needsScroll = (messageWidth > maxW);
        resetScroll();
        needsLayout = false;
    }

    if (!needsScroll) return; // Static text avoids costly loop redrawing

    int oldScrollX = scrollX;
    tick(currentMillis); 

    // Only draw if math changed our coordinate
    if (scrollX != oldScrollX) {
        int renderW = (width > 0) ? width : SCREEN_WIDTH;
        int renderH = (height > 0) ? height : 12;

        display.setClipWindow(x, y, x + renderW, y + renderH);
        blankArea(display, x, y, renderW, renderH);

        display.setFont(font);
        display.drawStr(x - scrollX, y + renderH - 2, currentText);
        display.setMaxClipWindow();

        display.updateDisplayArea(x / 8, y / 8, (renderW + 7) / 8, (renderH + 7) / 8);
    }
}
