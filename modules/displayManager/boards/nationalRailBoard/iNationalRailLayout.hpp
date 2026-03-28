/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * Module: modules/displayManager/boards/nationalRailBoard/iNationalRailLayout.hpp
 * Description: Interface defining the superset of widgets available to National Rail layouts.
 *
 * Exported Functions/Classes:
 * - iNationalRailLayout: Base class for National Rail layouts.
 *   - tick(uint32_t currentMillis): Periodic logic update.
 *   - render(U8G2& display): Full screen render.
 *   - renderAnimationUpdate(U8G2& display, uint32_t currentMillis): High-speed animation pass.
 */

#pragma once
#ifndef I_NATIONAL_RAIL_LAYOUT_HPP
#define I_NATIONAL_RAIL_LAYOUT_HPP

#include "../interfaces/iBoardLayout.hpp"
#include <widgets/locationAndFiltersWidget.hpp>
#include <widgets/clockWidget.hpp>
#include <widgets/wifiStatusWidget.hpp>
#include <widgets/weatherWidget.hpp>
#include <widgets/serviceListWidget.hpp>
#include <widgets/scrollingMessagePoolWidget.hpp>
#include <widgets/labelWidget.hpp>

class appContext;

/**
 * @brief Base Layout class for National Rail layouts.
 *        Defines the "Superset" of widgets permitted on this board type.
 */
class iNationalRailLayout : public iBoardLayout {
public:
    // The Superset of widgets permitted on this board type (Public for Controller access)
    locationAndFiltersWidget locationAndFilters; // Combined station identity and meta-data
    weatherWidget weather;          // Weather status
    wifiStatusWidget wifiWarning;   // Connectivity status
    clockWidget sysClock;           // System clock
    serviceListWidget row0Widget;   // Specialized Row 0 widget
    serviceListWidget servicesWidget; // Main services list
    scrollingMessagePoolWidget msgWidget; // Ticker tape messages
    labelWidget noDataLabel;        // Fallback info label

    /**
     * @brief Constructor for the National Rail view.
     * @param context Pointer to global app context.
     */
    iNationalRailLayout(appContext* context);

    virtual ~iNationalRailLayout() = default;

    /**
     * @brief Periodic logic update for all widgets.
     * @param currentMillis Current system time in milliseconds.
     */
    virtual void tick(uint32_t currentMillis) override {
        locationAndFilters.tick(currentMillis);
        weather.tick(currentMillis);
        wifiWarning.tick(currentMillis);
        sysClock.tick(currentMillis);
        row0Widget.tick(currentMillis);
        servicesWidget.tick(currentMillis);
        msgWidget.tick(currentMillis);
        noDataLabel.tick(currentMillis);
    }

    /**
     * @brief Full screen render pass.
     * @param display Reference to U8g2 context.
     */
    virtual void render(U8G2& display) override {
        locationAndFilters.render(display);
        weather.render(display);
        wifiWarning.render(display);
        sysClock.render(display);
        row0Widget.render(display);
        servicesWidget.render(display);
        msgWidget.render(display);
        noDataLabel.render(display);
    }

    /**
     * @brief Targeted animation updates for dynamic widgets.
     * @param display Reference to U8g2 context.
     * @param currentMillis Current system time in milliseconds.
     */
    virtual void renderAnimationUpdate(U8G2& display, uint32_t currentMillis) override {
        locationAndFilters.renderAnimationUpdate(display, currentMillis);
        weather.renderAnimationUpdate(display, currentMillis);
        wifiWarning.renderAnimationUpdate(display, currentMillis);
        sysClock.renderAnimationUpdate(display, currentMillis);
        row0Widget.renderAnimationUpdate(display, currentMillis);
        servicesWidget.renderAnimationUpdate(display, currentMillis);
        msgWidget.renderAnimationUpdate(display, currentMillis);
    }
};

#endif // I_NATIONAL_RAIL_LAYOUT_HPP
