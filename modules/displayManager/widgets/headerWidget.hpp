/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/gfxUtilities/headerWidget.hpp
 * Description: Renders the top bar of a departure board, including the location name.
 */

#ifndef HEADER_WIDGET_HPP
#define HEADER_WIDGET_HPP

#include "iGfxWidget.hpp"
#include "clockWidget.hpp"

class headerWidget : public iGfxWidget {
private:
    char title[64];
    char callingPoint[64];
    char platform[16];
    int timeOffset;
    bool showDate;
    bool needsLayout;

    char compositeTitle[150];
    int scrollX;
    int titleWidth;
    uint32_t lastScrollMs;
    bool needsScroll;

    clockWidget clock;

    void updateCompositeTitle();

public:
    headerWidget(int _x, int _y, int _w = -1, int _h = -1);

    void setTitle(const char* newTitle);
    void setCallingPoint(const char* newCp);
    void setPlatform(const char* newPlat);
    void setTimeOffset(int offset);
    void setShowDate(bool show);

    void resetScroll();

    clockWidget& getClock();

    void tick(uint32_t currentMillis) override;
    void render(U8G2& display) override;
    void renderAnimationUpdate(U8G2& display, uint32_t currentMillis) override;
};

#endif // HEADER_WIDGET_HPP
