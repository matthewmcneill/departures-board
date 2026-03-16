/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/widgets/wifiStatusWidget.cpp
 * Description: Implementation of the Wi-Fi status warning widget.
 *
 * Exported Functions/Classes:
 * - wifiStatusWidget: Graphical icon indicator for WiFi loss.
 *   - tick(): Monitors connection state and handles blinking timers.
 *   - render(): Draws the icon to the screen buffer.
 *   - renderAnimationUpdate(): Performs partial screen refresh for blinking.
 */

#include <widgets/wifiStatusWidget.hpp>
#include <WiFi.h>
#include "drawingPrimitives.hpp"

/**
 * @brief Constructor for the Wi-Fi status widget.
 * @param _x X coordinate for the icon.
 * @param _y Y coordinate for the icon.
 */
wifiStatusWidget::wifiStatusWidget(int _x, int _y)
    : iGfxWidget(_x, _y, 7, 7) {
    // Determine initial state
    wasConnected = (WiFi.status() == WL_CONNECTED);
}

/**
 * @brief Monitors the Wi-Fi connection state for changes.
 * @param currentMillis Current system time in milliseconds.
 */
void wifiStatusWidget::tick(uint32_t currentMillis) {
    if (!isVisible) return;
    
    bool currentlyConnected = (WiFi.status() == WL_CONNECTED);
    
    if (currentlyConnected) {
        disconnectTime = 0;
        blinkState = true;
    } else {
        if (disconnectTime == 0) disconnectTime = currentMillis;
        
        // Blink logic: after 30 seconds of disconnection
        if (currentMillis - disconnectTime > 30000) {
            if (currentMillis - lastBlinkMs > 1000) { // 1s toggle
                blinkState = !blinkState;
                lastBlinkMs = currentMillis;
            }
        } else {
            blinkState = true;
        }
    }
}

/**
 * @brief Renders the disconnected icon to the display buffer if not connected.
 * @param display Reference to the global U8g2 instance.
 */
void wifiStatusWidget::render(U8G2& display) {
    if (!isVisible) return;
    
    if (WiFi.status() != WL_CONNECTED && blinkState) {
        display.setFont(NatRailSmall9);
        display.drawStr(x, y, "\x7F");
    }
}

/**
 * @brief Immediately renders or clears the icon using an area update if connection state changed.
 * @param display Reference to the global U8g2 instance.
 * @param currentMillis Current system time in milliseconds.
 */
void wifiStatusWidget::renderAnimationUpdate(U8G2& display, uint32_t currentMillis) {
    if (!isVisible) return;
    
    bool currentlyConnected = (WiFi.status() == WL_CONNECTED);

    bool prevBlinkState = blinkState;
    tick(currentMillis);

    // If connection restored, or if blink state changed while disconnected
    if (currentlyConnected && !wasConnected) {
        wasConnected = true;
        // Erase the warning icon area
        display.setDrawColor(0);
        display.drawBox(x, y-6, width, height); 
        display.setDrawColor(1);
        display.updateDisplayArea(x / 8, (y-6) / 8, (width+7) / 8, (height+7) / 8);
    } 
    else if (!currentlyConnected) {
        if (wasConnected) {
            wasConnected = false;
            // Immediate draw on first disconnect
            display.setFont(NatRailSmall9);
            display.drawStr(x, y, "\x7F");
            display.updateDisplayArea(x / 8, (y-6) / 8, (width+7) / 8, (height+7) / 8);
        } else if (blinkState != prevBlinkState) {
            // Blink changed
            if (blinkState) {
                display.setFont(NatRailSmall9);
                display.drawStr(x, y, "\x7F");
            } else {
                display.setDrawColor(0);
                display.drawBox(x, y-6, width, height);
                display.setDrawColor(1);
            }
            display.updateDisplayArea(x / 8, (y-6) / 8, (width+7) / 8, (height+7) / 8);
        }
    }
}
