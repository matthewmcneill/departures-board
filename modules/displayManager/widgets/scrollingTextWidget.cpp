#include <departuresBoard.hpp>
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
 * Module: modules/displayManager/widgets/scrollingTextWidget.cpp
 * Description: Implementation of generic marquee scrolling logic.
 *
 * Exported Functions/Classes:
 * - scrollingTextWidget: [Class implementation]
 *   - setText(): Buffer assignment with change detection and layout invalidation.
 *   - resetScroll(): Rewinds pixel offset and resets timers.
 *   - tick(): Main progression math for pixel shifting and loop resets.
 *   - render(): Initial full-frame drawing with static/scroll detection.
 */

#include "scrollingTextWidget.hpp"
#include "drawingPrimitives.hpp"
#include <displayManager.hpp>

extern DisplayManager displayManager;

/**
 * @brief Initialize the scrolling text widget.
 * @param _x X coordinate.
 * @param _y Y coordinate.
 * @param _w Width (0 for full screen).
 * @param _h Height.
 * @param _font Optional font override.
 */
scrollingTextWidget::scrollingTextWidget(int _x, int _y, int _w, int _h, const uint8_t* _font)
    : iGfxWidget(_x, _y, _w, _h), scrollX(0), messageWidth(0), lastScrollMs(0), 
      needsScroll(false), needsLayout(true), font(_font), scrollFinished(false) {
    if (font == nullptr) font = NatRailSmall9;
    currentText[0] = '\0';
}

/**
 * @brief Assign new text to the widget.
 * Triggers a recalculation of layout metrics if the text has changed.
 * @param newText Null-terminated string.
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

    // --- Step 1: Initial Delay ---
    // Wait 3 seconds before starting the animation.
    if (lastScrollMs == 0) {
        lastScrollMs = currentMillis + 3000; 
        return;
    }

    if (currentMillis > lastScrollMs) {
        // --- Step 2: Pixel Shift ---
        scrollX++;
        int maxW = (width > 0) ? width : SCREEN_WIDTH;
        
        // --- Step 3: Loop/Exit Evaluation ---
        // Check if string has fully exited screen left (out of visibility)
        if (scrollX > messageWidth + 30) {
            scrollFinished = true; // Signal derived classes (e.g. MessagePool)
            scrollX = -maxW;       // Wrap completely around to right edge
        }
        
        lastScrollMs = currentMillis + 40; // Maintain ~25 FPS pacing
    }
}

/**
 * @brief Paints the widget entirely on-demand.
 */
/**
 * @brief Paints the widget entirely on-demand.
 * Dynamically determines if scrolling is required based on text width.
 * @param display U8g2 reference.
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

    U8g2StateSaver stateSaver(display);
    display.setFont(font);
    
    if (needsScroll) {
        // --- Arcane Logic ---
        // U8g2 coordinates are handled as 8-bit unsigned integers internally.
        // If x + renderW equals 256, it rolls over to 0, destroying the clipping zone.
        // We subtract 1 to ensure boundaries remain inclusive [0, 255].
        display.setClipWindow(x, y, x + renderW - 1, y + renderH - 1);
        drawText(display, currentText, x - scrollX, y, renderW + scrollX, renderH, TextAlign::LEFT, false);
    } else {
        // If it fits, we just center it statically
        drawText(display, currentText, x, y, renderW, renderH, TextAlign::CENTER, false);
        scrollFinished = true; // Automatically signal done since it never scrolls
    }
}

/**
 * @brief Targeted redraw for smooth marquee scrolling updates.
 * @param display U8g2 reference.
 * @param currentMillis Milliseconds since boot.
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

        U8g2StateSaver stateSaver(display);
        
        // --- Arcane Logic ---
        // Prevent 8-bit boundary wraps (256 -> 0) by using inclusive end coordinates.
        display.setClipWindow(x, y, x + renderW - 1, y + renderH - 1);
        blankArea(display, x, y, renderW, renderH);

        drawText(display, currentText, x - scrollX, y, renderW + scrollX, renderH, TextAlign::LEFT, false);

        display.updateDisplayArea(x / 8, y / 8, (renderW + 7) / 8, (renderH + 7) / 8);
    }
}
