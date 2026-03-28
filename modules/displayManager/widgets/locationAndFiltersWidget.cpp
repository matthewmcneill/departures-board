/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/widgets/locationAndFiltersWidget.cpp
 * Description: Implementation of consolidated metadata layout logic.
 */

#include "locationAndFiltersWidget.hpp"
#include "drawingPrimitives.hpp"
#include <fonts/fonts.hpp>
#include <string.h>

/**
 * @brief Construct a new consolidated header widget.
 */
locationAndFiltersWidget::locationAndFiltersWidget(int _x, int _y, int _w, int _h)
    : iGfxWidget(_x, _y, _w, _h), font(nullptr), 
      locationWidth(0), filtersWidth(0),
      scrollX(0), lastScrollMs(0),
      needsLayout(true), needsScroll(false) {
    
    memset(locationBuffer, 0, sizeof(locationBuffer));
    memset(filtersBuffer, 0, sizeof(filtersBuffer));
    if (font == nullptr) font = NatRailSmall9;
}

/**
 * @brief Set the location text.
 */
void locationAndFiltersWidget::setLocation(const char* text) {
    if (text == nullptr) {
        locationBuffer[0] = '\0';
    } else if (strcmp(locationBuffer, text) != 0) {
        strncpy(locationBuffer, text, sizeof(locationBuffer) - 1);
        locationBuffer[sizeof(locationBuffer) - 1] = '\0';
        needsLayout = true;
    }
}

/**
 * @brief Set the filters text.
 */
void locationAndFiltersWidget::setFilters(const char* text) {
    if (text == nullptr) {
        filtersBuffer[0] = '\0';
    } else if (strcmp(filtersBuffer, text) != 0) {
        strncpy(filtersBuffer, text, sizeof(filtersBuffer) - 1);
        filtersBuffer[sizeof(filtersBuffer) - 1] = '\0';
        needsLayout = true;
    }
}

/**
 * @brief Set the font for both fields.
 */
void locationAndFiltersWidget::setFont(const uint8_t* _font) {
    if (_font != font) {
        font = _font;
        needsLayout = true;
    }
}

/**
 * @brief Progress marquee scrolling for the filters text.
 */
void locationAndFiltersWidget::tick(uint32_t currentMillis) {
    if (!isVisible || !needsScroll) return;

    // --- Step 1: Initial Delay ---
    if (lastScrollMs == 0) {
        lastScrollMs = currentMillis + 3000; 
        return;
    }

    if (currentMillis > lastScrollMs) {
        // --- Step 2: Pixel Shift ---
        scrollX++;
        int filtersAreaWidth = width - locationWidth;
        
        // --- Step 3: Loop/Exit Evaluation ---
        if (scrollX > filtersWidth + 20) {
            scrollX = -filtersAreaWidth; // Wrap around to right edge
        }
        
        lastScrollMs = currentMillis + 40; // Maintain ~25 FPS pacing
    }
}

/**
 * @brief Paints the widget entirely on-demand.
 */
void locationAndFiltersWidget::render(U8G2& display) {
    if (!isVisible || !font) return;

    U8g2StateSaver stateSaver(display);
    display.setFont(font);

    // --- Step 1: Recalculate Layout if string or font changed ---
    if (needsLayout) {
        // Measure location width
        int rawLocationW = getStringWidth(display, locationBuffer) + 4; // added 1 for the frame pixel to avoid early truncation, and +3 for a bit of space.
        int maxLocationW = (width * 6) / 10; // 60% threshold
        
        if (rawLocationW > maxLocationW) {
            locationWidth = maxLocationW;
        } else {
            locationWidth = rawLocationW;
        }

        // Measure filters width
        filtersWidth = getStringWidth(display, filtersBuffer);
        int filtersAreaWidth = width - locationWidth;
        
        needsScroll = (filtersWidth > filtersAreaWidth);
        scrollX = 0;
        lastScrollMs = 0;
        needsLayout = false;
    }

    int filtersAreaWidth = width - locationWidth;

    // --- Step 2: Clear Area ---
    blankArea(display, x, y, width, height);

    // --- Step 3: Draw Components ---
    // Draw Location (truncated if necessary)
    drawText(display, locationBuffer, x, y, locationWidth, height, TextAlign::LEFT, true);

    // Draw Filters
    if (filtersBuffer[0] != '\0') {
        int filtersX = x + locationWidth;
        if (needsScroll) {
            display.setClipWindow(filtersX, y, filtersX + filtersAreaWidth - 1, y + height - 1);
            drawText(display, filtersBuffer, filtersX - scrollX, y, filtersWidth + scrollX, height, TextAlign::LEFT, false);
        } else {
            // Left-align if it fits
            drawText(display, filtersBuffer, filtersX, y, filtersAreaWidth, height, TextAlign::LEFT, false);
        }
    }
}

/**
 * @brief Optimized partial redraw for scrolling.
 */
void locationAndFiltersWidget::renderAnimationUpdate(U8G2& display, uint32_t currentMillis) {
    if (!isVisible || !needsScroll) return;

    int oldScrollX = scrollX;
    tick(currentMillis); 

    if (scrollX != oldScrollX) {
        U8g2StateSaver stateSaver(display);
        display.setFont(font);

        int filtersAreaWidth = width - locationWidth;
        int filtersX = x + locationWidth;
        
        display.setClipWindow(filtersX, y, filtersX + filtersAreaWidth - 1, y + height - 1);
        blankArea(display, filtersX, y, filtersAreaWidth, height);
        drawText(display, filtersBuffer, filtersX - scrollX, y, filtersWidth + scrollX, height, TextAlign::LEFT, false);

        display.updateDisplayArea(filtersX / 8, y / 8, (filtersAreaWidth + 7) / 8, (height + 7) / 8);
    }
}
