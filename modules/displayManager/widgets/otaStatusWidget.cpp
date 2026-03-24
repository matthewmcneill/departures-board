#include <fonts/fonts.hpp>
/*
 * Departures Board (c) 2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * Module: modules/displayManager/widgets/otaStatusWidget.cpp
 * Description: Implementation of the OTA status icon widget.
 */

#include "otaStatusWidget.hpp"
#include <appContext.hpp>
#include <displayManager.hpp>
#include "drawingPrimitives.hpp"

otaStatusWidget::otaStatusWidget(appContext* _context, int _x, int _y, int _w, int _h)
    : iGfxWidget(_x, _y, _w, _h), context(_context) {}

void otaStatusWidget::render(U8G2& display) {
    if (!isVisible || !context) return;

    if (!context->getDisplayManager().isOtaUpdateAvailable()) return;

    blankArea(display, x, y, width, height);
    display.setFont(NatRailSmall9);
    
    // Character 0xAB is the up-arrow/OTA symbol
    const char* icon = "\xAB";
    int iconW = display.getStrWidth(icon);
    
    int startX = x + (width - iconW) / 2;
    int baselineY = y + (height + display.getAscent() - display.getDescent()) / 2;
    
    display.drawStr(startX, baselineY, icon);
}
