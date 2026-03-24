#include <fonts/fonts.hpp>
/*
 * Departures Board (c) 2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * Module: modules/displayManager/widgets/weatherWidget.cpp
 * Description: Implementation of the weather icon widget.
 */

#include "weatherWidget.hpp"
#include <appContext.hpp>
#include <weatherClient.hpp>
#include <boards/interfaces/iDisplayBoard.hpp>
#include "drawingPrimitives.hpp"

extern const uint8_t WeatherIcons16[];

weatherWidget::weatherWidget(appContext* _context, int _x, int _y, int _w, int _h)
    : iGfxWidget(_x, _y, _w, _h), context(_context) {}

void weatherWidget::render(U8G2& display) {
    if (!isVisible || !context) return;

    iDisplayBoard* board = context->getDisplayManager().getCurrentBoard();
    if (!board) return;

    WeatherStatus& ws = board->getWeatherStatus();
    if (!ws.isValid()) return;

    blankArea(display, x, y, width, height);
    display.setFont(WeatherIcons16);
    
    char icon[2] = { ws.getIconChar(), '\0' };
    int iconW = display.getStrWidth(icon);
    
    int startX = x + (width - iconW) / 2;
    int baselineY = y + (height + display.getAscent() - display.getDescent()) / 2;
    
    display.drawStr(startX, baselineY, icon);
}
