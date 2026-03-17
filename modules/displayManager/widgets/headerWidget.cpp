/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/gfxUtilities/src/headerWidget.cpp
 * Description: Implementation of the headerWidget for top-bar rendering.
 */

#include "headerWidget.hpp"
#include "drawingPrimitives.hpp"
#include <time.h>

#include <appContext.hpp>
extern class appContext appContext;
extern const char* weekdays[];

headerWidget::headerWidget(int _x, int _y, int _w, int _h) 
    : iGfxWidget(_x, _y, _w, _h), timeOffset(0), showDate(false), needsLayout(true), scrollX(0), titleWidth(0), lastScrollMs(0), needsScroll(false),
      clock(&appContext.getTimeManager(), (_w > 0 ? _x + _w - 56 : SCREEN_WIDTH - 56), _y, 56, _h) {
    title[0] = '\0';
    callingPoint[0] = '\0';
    platform[0] = '\0';
    compositeTitle[0] = '\0';
}

void headerWidget::setTitle(const char* newTitle) {
    if (newTitle == nullptr) return;
    if (strcmp(title, newTitle) != 0) {
        strlcpy(title, newTitle, sizeof(title));
        updateCompositeTitle();
    }
}

void headerWidget::setCallingPoint(const char* newCp) {
    if (newCp == nullptr) return;
    if (strcmp(callingPoint, newCp) != 0) {
        strlcpy(callingPoint, newCp, sizeof(callingPoint));
        updateCompositeTitle();
    }
}

void headerWidget::setPlatform(const char* newPlat) {
    if (newPlat == nullptr) return;
    if (strcmp(platform, newPlat) != 0) {
        strlcpy(platform, newPlat, sizeof(platform));
        updateCompositeTitle();
    }
}

void headerWidget::setTimeOffset(int offset) {
    if (timeOffset != offset) {
        timeOffset = offset;
        // Optionally trigger title composite update iff layout tightens?
        // Right bound is dynamically calculated on render.
    }
}

void headerWidget::setShowDate(bool show) {
    showDate = show;
}

clockWidget& headerWidget::getClock() {
    return clock;
}

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

void headerWidget::resetScroll() {
    scrollX = 0;
    lastScrollMs = 0;
}

void headerWidget::tick(uint32_t currentMillis) {
    clock.tick(currentMillis);

    if (!isVisible || !needsScroll) return;

    if (lastScrollMs == 0) {
        lastScrollMs = currentMillis + 2000; // Initial pause
        return;
    }

    if (currentMillis > lastScrollMs) {
        scrollX++;
        int maxW = (width > 0) ? width : SCREEN_WIDTH;
        
        if (scrollX > titleWidth + 20) { // Scroll off and pause
            scrollX = -maxW; // Start from right
        }
        
        lastScrollMs = currentMillis + 40; // ~25fps scroll speed
    }
}

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

void headerWidget::render(U8G2& display) {
    if (!isVisible) return;

    if (needsLayout) {
        display.setFont(NatRailSmall9);
        titleWidth = getStringWidth(display, compositeTitle);
        int maxW = (width > 0) ? width : SCREEN_WIDTH;
        needsScroll = (titleWidth > (maxW - 45));
        resetScroll();
        needsLayout = false;
    }

    int renderW = (width > 0) ? width : SCREEN_WIDTH;
    int renderH = (height > 0) ? height : 14;

    blankArea(display, x, y, renderW, renderH);

    int rightBound = x + renderW;

    if (clock.getVisible()) {
        clock.render(display);
        rightBound -= 40; // Approx starting x of clock text
    }

    display.setFont(NatRailSmall9);

    if (timeOffset != 0) {
        char tOffset[20];
        snprintf(tOffset, sizeof(tOffset), "(+%d mins)", timeOffset);
        int w = display.getStrWidth(tOffset);
        rightBound -= (w + 4);
        display.drawStr(rightBound, y + renderH - 4, tOffset);
    } else if (showDate && appContext.getConfigManager().getConfig().dateEnabled) {
        char date[30];
        appContext.getTimeManager().updateCurrentTime();
        const struct tm& timeinfo = appContext.getTimeManager().getCurrentTime();
        byte wDay = timeinfo.tm_wday;
        byte mDay = timeinfo.tm_mday;
            snprintf(date, sizeof(date), "%s %02d", weekdays[wDay], mDay);
            int w = display.getStrWidth(date);
            rightBound -= (w + 4);
            display.drawStr(rightBound, y + renderH - 4, date);
    }

    if (needsScroll) {
        display.setClipWindow(x, y, rightBound - 2, y + renderH);
        display.drawStr(x - scrollX, y + renderH - 4, compositeTitle);
        display.setMaxClipWindow();
    } else {
        display.drawStr(x, y + renderH - 4, compositeTitle);
    }
}
