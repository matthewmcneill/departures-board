/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/gfxUtilities/wifiStatusWidget.hpp
 * Description: Renders a warning icon when the device loses Wi-Fi connectivity.
 *
 * Exported Functions/Classes:
 * - wifiStatusWidget: Class extending iGfxWidget to draw a disconnection icon.
 * - wifiStatusWidget::tick: Monitors connection state changes.
 * - wifiStatusWidget::render: Draws the icon if disconnected.
 * - wifiStatusWidget::renderAnimationUpdate: Non-blocking hardware redraw for the icon.
 */

#ifndef WIFI_STATUS_WIDGET_HPP
#define WIFI_STATUS_WIDGET_HPP

#include "iGfxWidget.hpp"

/**
 * @brief Widget to display a Wi-Fi disconnected warning icon.
 *        It monitors system state and only renders when offline.
 */
class wifiStatusWidget : public iGfxWidget {
private:
    bool wasConnected = true; // Tracks previous connection state to detect changes
    unsigned long disconnectTime = 0;
    bool blinkState = true;
    unsigned long lastBlinkMs = 0;

public:
    /**
     * @brief Constructor for the Wi-Fi status widget.
     * @param _x X coordinate for the icon.
     * @param _y Y coordinate for the icon.
     */
    wifiStatusWidget(int _x, int _y);

    /**
     * @brief Monitors the Wi-Fi connection state for changes.
     * @param currentMillis Current system time in milliseconds.
     */
    void tick(uint32_t currentMillis) override;

    /**
     * @brief Renders the disconnected icon to the display buffer if not connected.
     * @param display Reference to the global U8g2 instance.
     */
    void render(U8G2& display) override;

    /**
     * @brief Immediately renders or clears the icon using an area update if connection state changed.
     * @param display Reference to the global U8g2 instance.
     * @param currentMillis Current system time in milliseconds.
     */
    void renderAnimationUpdate(U8G2& display, uint32_t currentMillis) override;
};

#endif // WIFI_STATUS_WIDGET_HPP
