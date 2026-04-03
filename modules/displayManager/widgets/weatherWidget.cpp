#include <fonts/fonts.hpp>
/*
 * Departures Board (c) 2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * Module: modules/displayManager/widgets/weatherWidget.cpp
 * Description: Implementation of the weather icon widget.
 *
 * Exported Functions/Classes:
 * - weatherWidget: [Class implementation]
 *   - weatherWidget(): Constructor, defaults to 11pt icons.
 *   - render(): Fetches status from the current board and draws the glyph.
 */

#include "weatherWidget.hpp"
#include <appContext.hpp>
#include <weatherClient.hpp>
#include <boards/interfaces/iDisplayBoard.hpp>
#include "drawingPrimitives.hpp"

extern const uint8_t WeatherIcons16[];
extern const uint8_t WeatherIcons11[];

/**
 * @brief Initialize the weather icon widget.
 * @param _context Pointer to the application context.
 * @param _x X coordinate.
 * @param _y Y coordinate.
 * @param _w width.
 * @param _h height.
 */
weatherWidget::weatherWidget(appContext* _context, int _x, int _y, int _w, int _h)
    : iGfxWidget(_x, _y, _w, _h), context(_context), font(WeatherIcons11) {}

/**
 * @brief Renders the current weather glyph if valid status exists.
 * @param display U8G2 reference.
 */
void weatherWidget::render(U8G2& display) {
    if (!isVisible || !context) return;

    iDisplayBoard* board = context->getDisplayManager().getCurrentBoard();
    if (!board) return;

    WeatherStatus& ws = board->getWeatherStatus();
    if (!ws.isValid()) return;

    blankArea(display, x, y, width, height);
    
    char icon[2] = { ws.getIconChar(), '\0' };
    drawText(display, icon, x, y, width, height, TextAlign::CENTER, false, font);
}
