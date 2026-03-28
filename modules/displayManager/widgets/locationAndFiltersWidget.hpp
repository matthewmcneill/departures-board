/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/widgets/locationAndFiltersWidget.hpp
 * Description: A consolidated header widget that dynamically allocates space 
 *              to a fixed (truncated) location label and a scrolling filter info text.
 *
 * Exported Functions/Classes:
 * - locationAndFiltersWidget: Consolidated metadata component.
 *   - setLocation(): Update the fixed station name.
 *   - setFilters(): Update the scrolling filter string.
 *   - setFont(): Assign typography to both elements.
 *   - tick(): Update scrolling animation logic.
 *   - render(): Draw combined components with dynamic spacing.
 */

#ifndef LOCATION_AND_FILTERS_WIDGET_HPP
#define LOCATION_AND_FILTERS_WIDGET_HPP

#include "iGfxWidget.hpp"

/**
 * @brief A hybrid widget that manages both a fixed location name and 
 *        a scrolling filter string. It calculates the width of the 
 *        location and truncates it if it exceeds 60% of available space.
 */
class locationAndFiltersWidget : public iGfxWidget {
private:
    char locationBuffer[128];     // Buffer for the fixed station name
    char filtersBuffer[512];      // Buffer for the marquee scrolling filters
    const uint8_t* font;          // Shared typography for both fields
    
    int locationWidth;            // Rendered width of the location string
    int filtersWidth;             // Rendered width of the filters string
    int scrollX;                  // Marquee pixel offset for filters
    uint32_t lastScrollMs;        // Timing reference for animation
    
    bool needsLayout;             // Flag to trigger width recalc
    bool needsScroll;             // Flag if filters exceed remaining space

public:
    /**
     * @brief Construct a new consolidated header widget.
     * @param _x X coordinate.
     * @param _y Y coordinate (top-left).
     * @param _w Total width of the combined area.
     * @param _h Height of the combined area.
     */
    locationAndFiltersWidget(int _x, int _y, int _w, int _h);

    /**
     * @brief Set the location text.
     * @param text Station name string.
     */
    void setLocation(const char* text);

    /**
     * @brief Set the filters text.
     * @param text Filter/via information string.
     */
    void setFilters(const char* text);

    /**
     * @brief Set the font for both fields.
     * @param _font U8G2 font pointer.
     */
    void setFont(const uint8_t* _font);

    /**
     * @brief Update scrolling animation state.
     * @param currentMillis System runtime.
     */
    void tick(uint32_t currentMillis) override;

    /**
     * @brief Render both components into the display buffer.
     * @param display U8g2 reference.
     */
    void render(U8G2& display) override;

    /**
     * @brief High-speed partial redraw for scrolling filters.
     * @param display U8g2 reference.
     * @param currentMillis System runtime.
     */
    void renderAnimationUpdate(U8G2& display, uint32_t currentMillis) override;
};

#endif // LOCATION_AND_FILTERS_WIDGET_HPP
