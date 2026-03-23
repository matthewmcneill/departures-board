/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * Module: modules/displayManager/boards/nationalRailBoard/iNationalRailLayout.hpp
 * Description: Interface defining the superset of widgets available to National Rail layouts.
 */

#pragma once
#ifndef I_NATIONAL_RAIL_LAYOUT_HPP
#define I_NATIONAL_RAIL_LAYOUT_HPP

#include "../interfaces/iBoardLayout.hpp"
#include <widgets/headerWidget.hpp>
#include <widgets/serviceListWidget.hpp>
#include <widgets/scrollingMessagePoolWidget.hpp>
#include <widgets/clockWidget.hpp>
#include <widgets/labelWidget.hpp>

class appContext;

/**
 * @brief Base Layout class for National Rail layouts.
 *        Defines the "Superset" of widgets permitted on this board type.
 */
class iNationalRailLayout : public iBoardLayout {
public:
    // The Superset of widgets permitted on this board type (Public for Controller access)
    headerWidget headWidget;
    serviceListWidget row0Widget;   // Specialized Row 0 widget
    serviceListWidget servicesWidget;
    scrollingMessagePoolWidget msgWidget;
    clockWidget sysClock;
    labelWidget noDataLabel;

    /**
     * @brief Constructor for the National Rail view.
     *        Initializes widgets but leaves coordinates to derived classes.
     */
    iNationalRailLayout(appContext* context);

    /**
     * @brief Virtual destructor.
     */
    virtual ~iNationalRailLayout() = default;

    /**
     * @brief Periodic logic update for all widgets (Header, Specialized Rows, List, Clock).
     * @param currentMillis Current system time in milliseconds.
     */
    virtual void tick(uint32_t currentMillis) override {
        headWidget.tick(currentMillis);
        row0Widget.tick(currentMillis);
        servicesWidget.tick(currentMillis);
        msgWidget.tick(currentMillis);
        sysClock.tick(currentMillis);
        noDataLabel.tick(currentMillis);
    }

    virtual void render(U8G2& display) override {
        headWidget.render(display);
        row0Widget.render(display);
        servicesWidget.render(display);
        msgWidget.render(display);
        sysClock.render(display);
        noDataLabel.render(display);
    }
       /**
     * @brief Targeted animation updates for all dynamic widgets.
     * @param display Reference to U8g2.
     * @param currentMillis Current system time in milliseconds.
     */
    virtual void renderAnimationUpdate(U8G2& display, uint32_t currentMillis) override {
        headWidget.renderAnimationUpdate(display, currentMillis);
        row0Widget.renderAnimationUpdate(display, currentMillis);
        servicesWidget.renderAnimationUpdate(display, currentMillis);
        msgWidget.renderAnimationUpdate(display, currentMillis);
        sysClock.renderAnimationUpdate(display, currentMillis);
    }
};

#endif // I_NATIONAL_RAIL_LAYOUT_HPP
