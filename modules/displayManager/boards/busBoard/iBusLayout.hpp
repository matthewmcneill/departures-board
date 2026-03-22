/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * Module: modules/displayManager/boards/busBoard/iBusLayout.hpp
 * Description: Interface defining the superset of widgets available to Bus layouts.
 */

#pragma once
#ifndef I_BUS_LAYOUT_HPP
#define I_BUS_LAYOUT_HPP

class appContext;
#include "../interfaces/iBoardLayout.hpp"
#include <widgets/headerWidget.hpp>
#include <widgets/serviceListWidget.hpp>
#include <widgets/scrollingMessagePoolWidget.hpp>
#include <widgets/labelWidget.hpp>

/**
 * @brief Base Layout class for Bus layouts.
 */
class iBusLayout : public iBoardLayout {
public:
    headerWidget headWidget;
    serviceListWidget servicesWidget;
    scrollingMessagePoolWidget msgWidget;
    labelWidget noDataLabel;

    iBusLayout(appContext* context) 
        : iBoardLayout(context),
          headWidget(0, 0, 0, 0),
          servicesWidget(0, 0, 0, 0),
          msgWidget(0, 0, 0, 0),
          noDataLabel(0, 0, 0, 0) {
              noDataLabel.setVisible(false);
          }

    /**
     * @brief Virtual destructor.
     */
    virtual ~iBusLayout() = default;
    
    /**
     * @brief Periodic logic update for all layouts (Header, List, Message).
     * @param currentMillis Current system time in milliseconds.
     */
    virtual void tick(uint32_t currentMillis) override {
        headWidget.tick(currentMillis);
        servicesWidget.tick(currentMillis);
        msgWidget.tick(currentMillis);
        noDataLabel.tick(currentMillis);
    }

    /**
     * @brief Full screen render for the default Bus board logic.
     * @param display Reference to U8g2.
     */
    virtual void render(U8G2& display) override {
        headWidget.render(display);
        servicesWidget.render(display);
        msgWidget.render(display);
        noDataLabel.render(display);
    }
    
    /**
     * @brief Targeted animation updates for scrolling headers and messages.
     * @param display Reference to U8g2.
     * @param currentMillis Current system time in milliseconds.
     */
    virtual void renderAnimationUpdate(U8G2& display, uint32_t currentMillis) override {
        headWidget.renderAnimationUpdate(display, currentMillis);
        servicesWidget.renderAnimationUpdate(display, currentMillis);
        msgWidget.renderAnimationUpdate(display, currentMillis);
    }
};

#endif // I_BUS_LAYOUT_HPP
