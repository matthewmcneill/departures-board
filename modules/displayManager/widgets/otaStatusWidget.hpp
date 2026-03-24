/*
 * Departures Board (c) 2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * Module: modules/displayManager/widgets/otaStatusWidget.hpp
 * Description: Renders the OTA update available icon.
 */

#ifndef OTA_STATUS_WIDGET_HPP
#define OTA_STATUS_WIDGET_HPP

#include "iGfxWidget.hpp"

class appContext;

class otaStatusWidget : public iGfxWidget {
private:
    appContext* context;

public:
    otaStatusWidget(appContext* _context, int _x, int _y, int _w = 12, int _h = 12);

    void tick(uint32_t currentMillis) override {}
    void render(U8G2& display) override;
};

#endif // OTA_STATUS_WIDGET_HPP
