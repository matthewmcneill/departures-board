/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * Module: modules/displayManager/boards/tflBoard/iTflLayout.hpp
 * Description: Interface defining the superset of widgets available to TfL layouts.
 */

#pragma once
#ifndef I_TFL_LAYOUT_HPP
#define I_TFL_LAYOUT_HPP

class appContext;
#include "../interfaces/iBoardLayout.hpp"
#include <widgets/headerWidget.hpp>
#include <widgets/serviceListWidget.hpp>
#include <widgets/scrollingMessagePoolWidget.hpp>

/**
 * @brief Base Layout class for TfL layouts (Tube).
 */
class iTflLayout : public iBoardLayout {
public:
    headerWidget headWidget;
    serviceListWidget servicesWidget;
    scrollingMessagePoolWidget msgWidget;

    iTflLayout(appContext* context) 
        : iBoardLayout(context),
          headWidget(0, 0, 0, 0),
          servicesWidget(0, 0, 0, 0),
          msgWidget(0, 0, 0, 0) {}

    virtual ~iTflLayout() = default;

    virtual void tick(uint32_t currentMillis) override {
        headWidget.tick(currentMillis);
        servicesWidget.tick(currentMillis);
        msgWidget.tick(currentMillis);
    }

    virtual void render(U8G2& display) override {
        headWidget.render(display);
        servicesWidget.render(display);
        msgWidget.render(display);
    }
    
    virtual void renderAnimationUpdate(U8G2& display, uint32_t currentMillis) override {
        headWidget.renderAnimationUpdate(display, currentMillis);
        servicesWidget.renderAnimationUpdate(display, currentMillis);
        msgWidget.renderAnimationUpdate(display, currentMillis);
    }
};

#endif // I_TFL_LAYOUT_HPP
