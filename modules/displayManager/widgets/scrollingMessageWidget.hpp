/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/gfxUtilities/scrollingMessageWidget.hpp
 * Description: Horizontal marquee scroller for calling points and alerts.
 */

#ifndef SCROLLING_MESSAGE_WIDGET_HPP
#define SCROLLING_MESSAGE_WIDGET_HPP

#include "iGfxWidget.hpp"

class scrollingMessageWidget : public iGfxWidget {
private:
    char message[512];
    int scrollX;
    int messageWidth;
    uint32_t lastScrollMs;
    bool needsScroll;
    bool needsLayout;
    const uint8_t* font;

public:
    scrollingMessageWidget(int _x, int _y, int _w = -1, int _h = -1, const uint8_t* _font = nullptr);

    void setMessage(const char* newMessage);
    void resetScroll();

    void tick(uint32_t currentMillis) override;
    void render(U8G2& display) override;
    void renderAnimationUpdate(U8G2& display, uint32_t currentMillis) override;
};

#endif // SCROLLING_MESSAGE_WIDGET_HPP
