/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * Module: modules/displayManager/boards/tflBoard/iTflLayout.hpp
 * Description: Interface defining the superset of widgets available to TfL layouts.
 *
 * Exported Functions/Classes:
 * - iTflLayout: [Interface] Base class for TfL Tube layouts.
 *   - iTflLayout(): Constructor with context injection.
 *   - tick(): View-specific timing logic.
 *   - render(): Full screen rendering.
 *   - renderAnimationUpdate(): High-speed animation pass.
 */

#pragma once
#ifndef I_TFL_LAYOUT_HPP
#define I_TFL_LAYOUT_HPP

class appContext;
#include "../interfaces/iBoardLayout.hpp"
#include <widgets/locationAndFiltersWidget.hpp>
#include <widgets/serviceListWidget.hpp>
#include <widgets/scrollingMessagePoolWidget.hpp>
#include <widgets/labelWidget.hpp>
#include <widgets/wifiStatusWidget.hpp>
#include <widgets/weatherWidget.hpp>
#include <widgets/clockWidget.hpp>

/**
 * @brief Base Layout class for TfL layouts (Tube).
 */
class iTflLayout : public iBoardLayout {
public:
    locationAndFiltersWidget locationAndFilters;
    weatherWidget weather;
    wifiStatusWidget wifiWarning;
    clockWidget sysClock;
    serviceListWidget row0Widget;
    serviceListWidget servicesWidget;
    scrollingMessagePoolWidget msgWidget;
    labelWidget noDataLabel;

    iTflLayout(appContext* context);

    /**
     * @brief Virtual destructor.
     */
    virtual ~iTflLayout() = default;

    /**
     * @brief Periodic logic update for all layouts (Header, List, Message).
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
     * @brief Full screen render for the default TfL board logic.
     * @param display Reference to U8g2.
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
     * @brief Targeted animation updates for scrolling headers and messages.
     * @param display Reference to U8g2.
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

#endif // I_TFL_LAYOUT_HPP
