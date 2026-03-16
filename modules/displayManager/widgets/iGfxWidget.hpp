/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/gfxUtilities/iGfxWidget.hpp
 * Description: Base interface for reusable graphical components.
 */

#ifndef I_GFX_WIDGET_HPP
#define I_GFX_WIDGET_HPP

#include <U8g2lib.h>
#include <stdint.h>

/**
 * @brief Abstract base class for all UI widgets.
 *        Manages its own position, optional size, and visibility.
 */
class iGfxWidget {
protected:
    int x, y;
    int width, height;
    bool isVisible = true;

public:
    /**
     * @brief Constructor for the widget.
     * @param _x X coordinate on the OLED.
     * @param _y Y coordinate on the OLED.
     * @param _w Width in pixels (-1 for dynamic or full width).
     * @param _h Height in pixels (-1 for dynamic or full height).
     */
    iGfxWidget(int _x, int _y, int _w = -1, int _h = -1) 
        : x(_x), y(_y), width(_w), height(_h) {}

    virtual ~iGfxWidget() = default;

    /**
     * @brief Sets the visibility of the widget.
     * @param visible True to render, false to skip.
     */
    void setVisible(bool visible) { isVisible = visible; }

    /**
     * @brief Checks if the widget is currently visible.
     */
    bool getVisible() const { return isVisible; }

    /**
     * @brief Periodic logic update for state math and timing.
     *        USE THIS 99% of the time for standard animations (e.g. scrollX++).
     *        This is called by the main loop and safely precedes a synchronized full-screen render.
     * @param currentMillis Current system time in milliseconds.
     */
    virtual void tick(uint32_t currentMillis) = 0;

    /**
     * @brief Renders the widget to the display buffer if visible.
     * @param display Reference to the global U8g2 instance.
     */
    virtual void render(U8G2& display) = 0;

    /**
     * @brief Emergency non-blocking hardware-accelerated update ("Escape Hatch").
     *        ONLY use this if the physical screen must move during a long blocking operation (e.g. Wi-Fi download).
     *        WARNING: Implementations MUST constrain their visual update using `display.updateDisplayArea`
     *        to a tiny local bounding box to avoid corrupting the frozen screen buffer or saturating the SPI bus.
     *        
     *        DEDUPLICATION PATTERN: Implementations should unconditionally call `tick(currentMillis)` 
     *        first to calculate state changes, and only trigger `updateDisplayArea()` if `tick` 
     *        actually modified the widget's mathematical state.
     * @param display Reference to the global U8g2 instance.
     * @param currentMillis Current system time in milliseconds.
     */
    virtual void renderAnimationUpdate(U8G2& display, uint32_t currentMillis) {}
};

#endif // I_GFX_WIDGET_HPP
