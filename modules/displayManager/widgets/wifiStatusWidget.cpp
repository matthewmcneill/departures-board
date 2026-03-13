/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/gfxUtilities/wifiStatusWidget.cpp
 * Description: Implementation of the Wi-Fi status warning widget.
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
    
    // We only track the current and previous states during normal loop traversal
    bool currentlyConnected = (WiFi.status() == WL_CONNECTED);
    
    // The renderAnimationUpdate will act upon differences
}

/**
 * @brief Renders the disconnected icon to the display buffer if not connected.
 * @param display Reference to the global U8g2 instance.
 */
void wifiStatusWidget::render(U8G2& display) {
    if (!isVisible) return;
    
    if (WiFi.status() != WL_CONNECTED) {
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

    // State changed from disconnected to connected
    if (currentlyConnected && !wasConnected) {
        wasConnected = true;
        // Erase the warning icon area
        display.setDrawColor(0);
        display.drawBox(x, y-6, width, height); // Adjusting for font baseline
        display.setDrawColor(1);
        display.updateDisplayArea(x / 8, (y-6) / 8, (width+7) / 8, (height+7) / 8);
    } 
    // State changed from connected to disconnected
    else if (!currentlyConnected && wasConnected) {
        wasConnected = false;
        // Draw the warning icon
        display.setFont(NatRailSmall9);
        display.drawStr(x, y, "\x7F");
        display.updateDisplayArea(x / 8, (y-6) / 8, (width+7) / 8, (height+7) / 8);
    }
}
