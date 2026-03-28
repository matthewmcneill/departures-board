/*
 * Departures Board (c) 2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * Module: modules/displayManager/widgets/weatherWidget.hpp
 * Description: Renders the current weather icon from the injected appContext.
 */

#ifndef WEATHER_WIDGET_HPP
#define WEATHER_WIDGET_HPP

#include "iGfxWidget.hpp"

class appContext;

class weatherWidget : public iGfxWidget {
private:
    appContext* context;
    const uint8_t* font;

public:
    weatherWidget(appContext* _context, int _x, int _y, int _w = 16, int _h = 16);

    void setFont(const uint8_t* _font) { font = _font; }
    void tick(uint32_t currentMillis) override {}
    void render(U8G2& display) override;
};

#endif // WEATHER_WIDGET_HPP
