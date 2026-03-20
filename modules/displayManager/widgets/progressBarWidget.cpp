/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/gfxUtilities/src/progressBarWidget.cpp
 * Description: Implementation of the progress bar widget.
 */

#include "progressBarWidget.hpp"
#include "drawingPrimitives.hpp"
#include <displayManager.hpp>
#include <string.h>

progressBarWidget::progressBarWidget(int _x, int _y, int _w, int _h, const uint8_t* _font)
    : iGfxWidget(_x, _y, _w, _h), currentPercent(0), targetPercent(0), startPercent(0), animStartTime(0), animDurationMs(0), oldPercent(0), textChanged(false), showPercentText(false), font(_font) {
    if (font == nullptr) font = NatRailSmall9;
    message[0] = '\0';
}

void progressBarWidget::setMessage(const char* newMessage) {
    if (newMessage == nullptr) return;
    if (strcmp(message, newMessage) != 0) {
        strlcpy(message, newMessage, sizeof(message));
        textChanged = true;
    }
}

void progressBarWidget::setMessage(const __FlashStringHelper* newMessage) {
    char buf[256];
    strcpy_P(buf, (const char*)newMessage);
    setMessage(buf);
}

void progressBarWidget::setShowPercentText(bool show) {
    if (showPercentText != show) {
        showPercentText = show;
        textChanged = true;
    }
}

void progressBarWidget::setPercent(int percent, uint32_t durationMs) {
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    
    animDurationMs = durationMs;
    
    if (animDurationMs > 0) {
        startPercent = currentPercent;
        targetPercent = percent;
        animStartTime = millis();
    } else {
        currentPercent = percent;
        targetPercent = percent;
    }
}

void progressBarWidget::tick(uint32_t currentMillis) {
    if (!isVisible) return;
    
    if (animDurationMs > 0 && currentPercent != targetPercent) {
        uint32_t elapsed = currentMillis - animStartTime;
        if (elapsed >= animDurationMs) {
            currentPercent = targetPercent;
        } else {
            float progress = (float)elapsed / animDurationMs;
            currentPercent = startPercent + (targetPercent - startPercent) * progress;
        }
    }
}

void progressBarWidget::renderAnimationUpdate(U8G2& display, uint32_t currentMillis) {
    if (!isVisible) return;

    tick(currentMillis); // Standard deduplication practice

    if (currentPercent != oldPercent || textChanged) {
        oldPercent = currentPercent;
        textChanged = false;

        render(display);
        
        int renderW = (width > 0) ? width : SCREEN_WIDTH;
        int renderH = (height > 0) ? height : 20; // Enough for text, number, and bar
        display.updateDisplayArea(x / 8, y / 8, (renderW + 7) / 8, (renderH + 7) / 8);
    }
}

void progressBarWidget::render(U8G2& display) {
    if (!isVisible) return;

    int renderW = (width > 0) ? width : SCREEN_WIDTH;
    int renderH = (height > 0) ? height : 20;

    // Clear the bounding box
    blankArea(display, x, y, renderW, renderH);

    display.setFont(font);
    
    // Draw the message text (centered or left aligned)
    if (message[0] != '\0' || showPercentText) {
        int textW = display.getStrWidth(message);
        
        char pctStr[10];
        pctStr[0] = '\0';
        int pctW = 0;
        
        if (showPercentText) {
            snprintf(pctStr, sizeof(pctStr), " %d%%", currentPercent);
            pctW = display.getStrWidth(pctStr);
        }
        
        // Draw centered message
        int totalW = textW + pctW;
        int startX = x + (renderW - totalW) / 2;
        if (startX < x) startX = x; // Trap boundary
        
        if (message[0] != '\0') {
            display.drawStr(startX, y + 8, message);
        }
        
        // Draw % aligned to right of message
        if (showPercentText) {
            display.drawStr(startX + textW, y + 8, pctStr);
        }
    }

    // Draw the bar itself at the bottom of the widget
    int barY = y + renderH - 4;
    int fill = (currentPercent * renderW) / 100;
    bool toggle = false;
    
    // Draw empty track background (optional, keeps it visually grounded)
    display.setDrawColor(0);
    display.drawBox(x, barY, renderW, 2);
    display.setDrawColor(1);

    // Draw filled track
    for (int i = 0; i < fill; i++) {
        if (toggle) {
            display.drawHVLine(x + i, barY, 1, 1);
            display.drawHVLine(x + i, barY + 1, 1, 0);
        } else {
            display.drawHVLine(x + i, barY, 1, 0);     
            display.drawHVLine(x + i, barY + 1, 1, 1);
        }
        toggle = !toggle;
    }
}
