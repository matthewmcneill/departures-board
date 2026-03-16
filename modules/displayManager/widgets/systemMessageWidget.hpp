/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/gfxUtilities/systemMessageWidget.hpp
 * Description: Full-screen message widget for alerts and setup screens.
 */

#ifndef SYSTEM_MESSAGE_WIDGET_HPP
#define SYSTEM_MESSAGE_WIDGET_HPP

#include "iGfxWidget.hpp"

class systemMessageWidget : public iGfxWidget {
private:
    char title[64];
    char lines[3][128];
    int numLines;

public:
    systemMessageWidget(int _x = 0, int _y = 0, int _w = 256, int _h = 64);

    void setMessage(const char* newTitle, const char* l1 = nullptr, const char* l2 = nullptr, const char* l3 = nullptr);
    
    void tick(uint32_t currentMillis) override { /* No animation usually */ }
    void render(U8G2& display) override;
};

#endif // SYSTEM_MESSAGE_WIDGET_HPP
