/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/gfxUtilities/clockWidget.hpp
 * Description: Renders the system clock with a configurable font and blinking colon.
 */

#ifndef CLOCK_WIDGET_HPP
#define CLOCK_WIDGET_HPP

#include "iGfxWidget.hpp"

class clockWidget : public iGfxWidget {
private:
    bool showColon;
    bool blinkEnabled;
    uint32_t lastBlinkMs;
    int lastMinute;
    const uint8_t* font;

public:
    /**
     * @brief Construct a new clock widget.
     * @param _x X position.
     * @param _y Y position.
     * @param _w Width (-1 for auto).
     * @param _h Height (-1 for auto).
     * @param _font Pointer to the u8g2 font to use.
     */
    clockWidget(int _x, int _y, int _w = -1, int _h = -1, const uint8_t* _font = nullptr);

    void setFont(const uint8_t* newFont);
    void setBlink(bool enable) { blinkEnabled = enable; showColon = true; }
    
    void tick(uint32_t currentMillis) override;
    void render(U8G2& display) override;
    void renderAnimationUpdate(U8G2& display, uint32_t currentMillis) override;
};

#endif // CLOCK_WIDGET_HPP
