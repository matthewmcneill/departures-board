/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/widgets/headerWidget.cpp
 * Description: Implementation of the header bar with title scrolling logic.
 */

#include "headerWidget.hpp"
#include "drawingPrimitives.hpp"
#include <time.h>

#include <appContext.hpp>
extern class appContext appContext;
extern const char* weekdays[];

/**
 * @brief Initialize the header with its internal clock and scroller state.
 */
headerWidget::headerWidget(int _x, int _y, int _w, int _h) 
    : iGfxWidget(_x, _y, _w, _h), timeOffset(0), showDate(false), needsLayout(true), scrollX(0), titleWidth(0), lastScrollMs(0), needsScroll(false),
      clock(&appContext.getTimeManager(), (_w > 0 ? _x + _w - 56 : SCREEN_WIDTH - 56), _y, 56, _h) {
    title[0] = '\0';
    callingPoint[0] = '\0';
    platform[0] = '\0';
    compositeTitle[0] = '\0';
}

/**
 * @brief Set the station or location name.
 * @param newTitle String of the new location.
 */
void headerWidget::setTitle(const char* newTitle) {
    if (newTitle == nullptr) return;
    if (strcmp(title, newTitle) != 0) {
        strlcpy(title, newTitle, sizeof(title));
        updateCompositeTitle();
    }
}

/**
 * @brief Set the intermediate calling point (e.g. "via Bristol").
 * @param newCp String of the via location.
 */
void headerWidget::setCallingPoint(const char* newCp) {
    if (newCp == nullptr) return;
    if (strcmp(callingPoint, newCp) != 0) {
        strlcpy(callingPoint, newCp, sizeof(callingPoint));
        updateCompositeTitle();
    }
}

/**
 * @brief Update the platform number or description.
 * @param newPlat Platform string.
 */
void headerWidget::setPlatform(const char* newPlat) {
    if (newPlat == nullptr) return;
    if (strcmp(platform, newPlat) != 0) {
        strlcpy(platform, newPlat, sizeof(platform));
        updateCompositeTitle();
    }
}

/**
 * @brief Set minutes to offset the displayed time.
 * @param offset Minutes as integer.
 */
void headerWidget::setTimeOffset(int offset) {
    if (timeOffset != offset) {
        timeOffset = offset;
        // Optionally trigger title composite update iff layout tightens?
        // Right bound is dynamically calculated on render.
    }
}

/**
 * @brief Control visibility of the current date.
 * @param show Boolean flag.
 */
void headerWidget::setShowDate(bool show) {
    showDate = show;
}

/**
 * @brief Accessor for the internal clock instance.
 * @return clockWidget reference.
 */
clockWidget& headerWidget::getClock() {
    return clock;
}

/**
 * @brief Build the composite title string for scrolling.
 */
void headerWidget::updateCompositeTitle() {
    if (callingPoint[0] != '\0') {
        if (platform[0] != '\0') {
            snprintf(compositeTitle, sizeof(compositeTitle), "%s to %s (Plat %s)", title, callingPoint, platform);
        } else {
            snprintf(compositeTitle, sizeof(compositeTitle), "%s to %s", title, callingPoint);
        }
    } else {
        if (platform[0] != '\0') {
            snprintf(compositeTitle, sizeof(compositeTitle), "%s (Plat %s)", title, platform);
        } else {
            strlcpy(compositeTitle, title, sizeof(compositeTitle));
        }
    }

    needsLayout = true;
}

/**
 * @brief Force the scrolling animation back to the start.
 */
void headerWidget::resetScroll() {
    scrollX = 0;
    lastScrollMs = 0;
}

/**
 * @brief Periodic logic for clock pulsing and scroller state management.
 * @param currentMillis Milliseconds since boot.
 */
void headerWidget::tick(uint32_t currentMillis) {
    if (!isVisible) return;
    
    // Delegate to the inner clock for blink timing
    clock.tick(currentMillis);

    // --- Step 1: Scroll Timer Evaluation ---
    if (!needsScroll) return;

    if (lastScrollMs == 0) {
        // Initial 2-second pause before starting the scroll
        lastScrollMs = currentMillis + 2000;
        return;
    }

    if (currentMillis > lastScrollMs) {
        // Pixel-shift the title
        scrollX++;
        int maxW = (width > 0) ? width : SCREEN_WIDTH;
        
        // Reset when the text has fully scrolled out of view plus margin
        if (scrollX > titleWidth + 20) {
            scrollX = -maxW; // Re-enter from the right
        }
        
        lastScrollMs = currentMillis + 40; // Maintain ~25 FPS scrolling
    }
}

/**
 * @brief High-speed redraw for smooth title scrolling.
 * @param display U8g2 reference.
 * @param currentMillis Milliseconds since boot.
 */
void headerWidget::renderAnimationUpdate(U8G2& display, uint32_t currentMillis) {
    if (!isVisible) return;

    if (needsLayout) {
        display.setFont(NatRailSmall9);
        titleWidth = getStringWidth(display, compositeTitle);
        int maxW = (width > 0) ? width : SCREEN_WIDTH;
        needsScroll = (titleWidth > (maxW - 45));
        resetScroll();
        needsLayout = false;
    }
    
    clock.renderAnimationUpdate(display, currentMillis);

    if (needsScroll) {
        int oldScrollX = scrollX;
        tick(currentMillis);
        
        if (scrollX != oldScrollX) {
            int renderW = (width > 0) ? width : SCREEN_WIDTH;
            int renderH = (height > 0) ? height : 14;
            
            display.setClipWindow(x, y, x + renderW - 56, y + renderH);
            blankArea(display, x, y, renderW - 56, renderH);

            display.setFont(NatRailSmall9);
            display.drawStr(x - scrollX, y + renderH - 4, compositeTitle);
            display.setMaxClipWindow();

            display.updateDisplayArea(x / 8, y / 8, (renderW - 56 + 7) / 8, (renderH + 7) / 8);
        }
    }
}

/**
 * @brief Full frame render for the header, including clock and title.
 * @param display U8g2 reference.
 */
void headerWidget::render(U8G2& display) {
    if (!isVisible) return;

    // --- Step 1: Layout Calculation ---
    // Recalculate if the title string actually requires scrolling.
    if (needsLayout) {
        display.setFont(NatRailSmall9);
        titleWidth = getStringWidth(display, compositeTitle);
        int maxW = (width > 0) ? width : SCREEN_WIDTH;
        needsScroll = (titleWidth > (maxW - 45)); // Reserve 45px for clock/date
        resetScroll();
        needsLayout = false;
    }

    int renderW = (width > 0) ? width : SCREEN_WIDTH;
    int renderH = (height > 0) ? height : 14;

    // Clear background
    blankArea(display, x, y, renderW, renderH);

    int rightBound = x + renderW;

    // --- Step 2: Render Right-Aligned Information ---
    if (clock.getVisible()) {
        clock.render(display);
        rightBound -= 56; // Standard clock slot width
    }

    display.setFont(NatRailSmall9);

    if (timeOffset != 0) {
        // Render time modifier (e.g. "+5 mins")
        char tOffset[20];
        snprintf(tOffset, sizeof(tOffset), "(+%d mins)", timeOffset);
        int w = display.getStrWidth(tOffset);
        rightBound -= (w + 4);
        display.drawStr(rightBound, y + renderH - 4, tOffset);
    } else if (showDate && appContext.getConfigManager().getConfig().dateEnabled) {
        // Render the day and date (e.g. "Fri 20")
        char date[30];
        appContext.getTimeManager().updateCurrentTime();
        const struct tm& timeinfo = appContext.getTimeManager().getCurrentTime();
        
        snprintf(date, sizeof(date), "%s %02d", weekdays[timeinfo.tm_wday], timeinfo.tm_mday);
        int w = display.getStrWidth(date);
        rightBound -= (w + 4);
        display.drawStr(rightBound, y + renderH - 4, date);
    }

    // --- Step 3: Draw the Title (Centered or Scrolled) ---
    if (needsScroll) {
        // Clip to the remaining area before the clock/date
        display.setClipWindow(x, y, rightBound - 2, y + renderH);
        display.drawStr(x - scrollX, y + renderH - 4, compositeTitle);
        display.setMaxClipWindow();
    } else {
        display.drawStr(x, y + renderH - 4, compositeTitle);
    }
}
