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

class appContext;

/**
 * @brief Base Layout class for National Rail layouts.
 *        Defines the "Superset" of widgets permitted on this board type.
 */
class iNationalRailLayout : public iBoardLayout {
public:
    // The Superset of widgets permitted on this board type (Public for Controller access)
    headerWidget headWidget;
    scrollingTextWidget row0Time;   // Specialized Row 0 widgets
    scrollingTextWidget row0Dest;   // Specialized Row 0 widgets
    serviceListWidget servicesWidget;
    scrollingMessagePoolWidget msgWidget;
    clockWidget sysClock;

    /**
     * @brief Constructor for the National Rail view.
     *        Initializes widgets but leaves coordinates to derived classes.
     */
    iNationalRailLayout(appContext* context);

    virtual ~iNationalRailLayout() = default;

    virtual void tick(uint32_t currentMillis) override {
        headWidget.tick(currentMillis);
        row0Time.tick(currentMillis);
        row0Dest.tick(currentMillis);
        servicesWidget.tick(currentMillis);
        msgWidget.tick(currentMillis);
        sysClock.tick(currentMillis);
    }

    virtual void render(U8G2& display) override {
        headWidget.render(display);
        row0Time.render(display);
        row0Dest.render(display);
        servicesWidget.render(display);
        msgWidget.render(display);
        sysClock.render(display);
    }
    
    virtual void renderAnimationUpdate(U8G2& display, uint32_t currentMillis) override {
        headWidget.renderAnimationUpdate(display, currentMillis);
        row0Time.renderAnimationUpdate(display, currentMillis);
        row0Dest.renderAnimationUpdate(display, currentMillis);
        servicesWidget.renderAnimationUpdate(display, currentMillis);
        msgWidget.renderAnimationUpdate(display, currentMillis);
        sysClock.renderAnimationUpdate(display, currentMillis);
    }
};

#endif // I_NATIONAL_RAIL_LAYOUT_HPP
