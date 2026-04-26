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
 * Module: modules/displayManager/widgets/progressBarWidget.cpp
 * Description: Implementation of animated progress bar rendering.
 *
 * Exported Functions/Classes:
 * - progressBarWidget: [Class implementation]
 *   - setMessage(): Updates the status label (supports PROGMEM).
 *   - setPercent(): Starts/snaps to a progress value with optional animation.
 *   - tick(): Interpolates current percentage during active animations.
 *   - render(): Paints the label and the checkered stipple progress bar.
 */

#include "progressBarWidget.hpp"
#include "drawingPrimitives.hpp"
#include <string.h>

/**
 * @brief Initialize the progress bar with its position and default font.
 * @param _x X coordinate.
 * @param _y Y coordinate.
 * @param _w Width (-1 for full screen).
 * @param _h Height (-1 for auto).
 * @param _font Optional font override.
 */
progressBarWidget::progressBarWidget(int _x, int _y, int _w, int _h, const uint8_t* _font)
    : iGfxWidget(_x, _y, _w, _h), currentPercent(0), targetPercent(0), startPercent(0), animStartTime(0), animDurationMs(0), oldPercent(0), textChanged(false), showPercentText(false), font(_font) {
    if (font == nullptr) font = NatRailSmall9;
    message[0] = '\0';
}

/**
 * @brief Set the status text displayed above the bar.
 * Matches existing content to avoid redundant redraws.
 * @param newMessage Pointer to the C-string.
 */
void progressBarWidget::setMessage(const char* newMessage) {
    if (newMessage == nullptr) return;
    if (strcmp(message, newMessage) != 0) {
        strlcpy(message, newMessage, sizeof(message));
        textChanged = true;
    }
}

/**
 * @brief Set the status text from a Flash-stored string (PROGMEM).
 * Reducer for memory-constrained strings.
 * @param newMessage Flash string pointer.
 */
void progressBarWidget::setMessage(const __FlashStringHelper* newMessage) {
    char buf[256];
    strcpy_P(buf, (const char*)newMessage);
    setMessage(buf);
}

/**
 * @brief Enable or disable numeric percentage text rendering.
 * @param show Boolean flag.
 */
void progressBarWidget::setShowPercentText(bool show) {
    if (showPercentText != show) {
        showPercentText = show;
        textChanged = true;
    }
}

/**
 * @brief Sets the target progress percentage.
 * Triggers interpolation logic on subsequent calls to tick().
 * @param percent Target value (0-100).
 * @param durationMs Animation length in milliseconds (0 = snap).
 */
void progressBarWidget::setPercent(int percent, uint32_t durationMs) {
    // Clamp to valid range
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    
    animDurationMs = durationMs;
    
    if (animDurationMs > 0) {
        // --- Step 1: Initialize Animation Path ---
        startPercent = currentPercent;
        targetPercent = percent;
        animStartTime = millis();
    } else {
        // --- Step 2: Instant State Transition ---
        currentPercent = percent;
        targetPercent = percent;
    }
}

/**
 * @brief Periodic logic for progress interpolation.
 * @param currentMillis Milliseconds since boot.
 */
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

/**
 * @brief Targeted redraw for smooth progress bar updates.
 * @param display U8g2 reference.
 * @param currentMillis Milliseconds since boot.
 */
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

/**
 * @brief Full frame render for the progress bar and label.
 * @param display U8g2 reference.
 */
void progressBarWidget::render(U8G2& display) {
    if (!isVisible) return;

    int renderW = (width > 0) ? width : SCREEN_WIDTH;
    int renderH = (height > 0) ? height : 20;

    // Clear the bounding box
    blankArea(display, x, y, renderW, renderH);

    U8g2StateSaver stateSaver(display);
    display.setFont(font);
    
    // Draw the message text (centered natively within bounds geometry)
    if (message[0] != '\0' || showPercentText) {
        char fullStr[128];
        
        // If message is empty but percent is shown, trim leading space
        if (showPercentText) {
            if (message[0] != '\0') {
                snprintf(fullStr, sizeof(fullStr), "%s %d%%", message, currentPercent);
            } else {
                snprintf(fullStr, sizeof(fullStr), "%d%%", currentPercent);
            }
        } else {
            strlcpy(fullStr, message, sizeof(fullStr));
        }

        drawText(display, fullStr, x, y, renderW, renderH - 4, TextAlign::CENTER, true);
    }

    // Draw the bar itself at the bottom of the widget
    int barY = y + renderH - 4;
    int fill = (currentPercent * renderW) / 100;
    bool toggle = false;
    
    // Draw empty track background (optional, keeps it visually grounded)
    display.setDrawColor(0);
    display.drawBox(x, barY, renderW, 2);
    display.setDrawColor(1);

    // --- Step 3: Draw Filled Track (Checkered Stippling) ---
    // Instead of a solid bar, we use every other pixel in a checkered pattern 
    // to give a classic LCD/DMD "shaded" look without alpha blending.
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
