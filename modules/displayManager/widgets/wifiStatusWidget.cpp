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
    : iGfxWidget(_x, _y, 11, 11) {
    // Determine initial state
    wasConnected = (WiFi.status() == WL_CONNECTED);
    lastRssiCategory = getRssiCategory();
}

int wifiStatusWidget::getRssiCategory() {
    if (WiFi.status() != WL_CONNECTED) return -1;
    int rssi = WiFi.RSSI();
    // In emulator or great signal, it's 0 to -50.
    if (rssi < -85) return 1;
    if (rssi < -75) return 2;
    if (rssi < -65) return 3;
    return 4;
}

/**
 * @brief Monitors the Wi-Fi connection state for changes.
 * @param currentMillis Current system time in milliseconds.
 */
void wifiStatusWidget::tick(uint32_t currentMillis) {
    if (!isVisible) return;
    
    bool currentlyConnected = (WiFi.status() == WL_CONNECTED);
    
    if (currentlyConnected) {
        // --- Reset State ---
        disconnectTime = 0;
        blinkState = true;
    } else {
        // --- Handle Disconnection ---
        if (disconnectTime == 0) disconnectTime = currentMillis;
        
        // Blink logic: after 30 seconds of disconnection, start the 1Hz blink
        if (currentMillis - disconnectTime > 30000) {
            if (currentMillis - lastBlinkMs > 1000) { 
                blinkState = !blinkState;
                lastBlinkMs = currentMillis;
            }
        } else {
            // Static icon for the first 30 seconds
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
    
    if (WiFi.status() != WL_CONNECTED) {
        if (blinkState) drawText(display, "X", x, y, width, height, TextAlign::CENTER, false, WifiIcons11);
    } else {
        int cat = getRssiCategory();
        const char* icon = "4";
        if (cat == 1) icon = "1";
        else if (cat == 2) icon = "2";
        else if (cat == 3) icon = "3";
        
        drawText(display, icon, x, y, width, height, TextAlign::CENTER, false, WifiIcons11);
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
    int currentRssiCat = getRssiCategory();

    bool prevBlinkState = blinkState;
    tick(currentMillis);

    bool needsRedraw = false;

    // --- Transition: Restored ---
    if (currentlyConnected && !wasConnected) {
        wasConnected = true;
        lastRssiCategory = currentRssiCat;
        needsRedraw = true;
    } 
    // --- Transition: RSSI Changed while Connected ---
    else if (currentlyConnected && wasConnected && currentRssiCat != lastRssiCategory) {
        lastRssiCategory = currentRssiCat;
        needsRedraw = true;
    }
    // --- Transition: Offline / Blinking ---
    else if (!currentlyConnected) {
        if (wasConnected) {
            // First time we detected the swap
            wasConnected = false;
            lastRssiCategory = -1;
            needsRedraw = true;
        } else if (blinkState != prevBlinkState) {
            // Blinking timer triggered a swap
            needsRedraw = true;
        }
    }

    if (needsRedraw) {
        blankArea(display, x, y, width, height);
        
        if (!currentlyConnected) {
            if (blinkState) drawText(display, "X", x, y, width, height, TextAlign::CENTER, false, WifiIcons11);
        } else {
            const char* icon = "4";
            if (currentRssiCat == 1) icon = "1";
            else if (currentRssiCat == 2) icon = "2";
            else if (currentRssiCat == 3) icon = "3";
            drawText(display, icon, x, y, width, height, TextAlign::CENTER, false, WifiIcons11);
        }
        display.updateDisplayArea(x / 8, y / 8, (width+7) / 8, (height+7) / 8);
    }
}
