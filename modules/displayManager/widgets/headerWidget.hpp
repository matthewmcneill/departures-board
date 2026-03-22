/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/widgets/headerWidget.hpp
 * Description: Renders the top-level status bar for a board. Includes the primary 
 *              location name, calling points/via destinations, platform info, 
 *              and an integrated real-time clock. Supports automatic horizontal 
 *              text scrolling for long titles.
 *
 * Exported Functions/Classes:
 * - headerWidget: Graphics widget for board identity and system time.
 *   - setTitle(): Update the primary location string.
 *   - setCallingPoint(): Update the secondary destination string.
 *   - setPlatform(): Update the platform occupancy/tracking string.
 *   - setTimeOffset(): Apply a local timezone/offset adjustment.
 *   - setShowDate(): Toggle the visibility of the current date.
 *   - resetScroll(): Force the title scroller back to index zero.
 *   - getClock(): Direct access to the internal clock widget.
 *   - tick(): Logic update for scroll and clock.
 *   - render(): Primary drawing method.
 *   - renderAnimationUpdate(): Targeted scroller animation.
 */
#ifndef HEADER_WIDGET_HPP
#define HEADER_WIDGET_HPP

#include "iGfxWidget.hpp"
#include "clockWidget.hpp"

class headerWidget : public iGfxWidget {
private:
    char title[64];
    char callingPoint[64];
    char platform[16];
    int timeOffset;
    bool showDate;
    bool needsLayout;

    char compositeTitle[150];
    int scrollX;
    int titleWidth;
    uint32_t lastScrollMs;
    bool needsScroll;

    clockWidget clock;

    /**
     * @brief Internal helper to rebuild the scrolling string from title components.
     */
    void updateCompositeTitle();

public:
    /**
     * @brief Construct a new header widget at specific coordinates.
     * @param _x X coordinate in pixels.
     * @param _y Y coordinate in pixels.
     * @param _w Optional width (defaults to SCREEN_WIDTH).
     * @param _h Optional height.
     */
    headerWidget(int _x, int _y, int _w = -1, int _h = -1);

    /**
     * @brief Set the primary location name to render.
     * @param newTitle The new string text.
     */
    void setTitle(const char* newTitle);

    /**
     * @brief Set the secondary "via" or "calling at" location.
     * @param newCp The string destination text.
     */
    void setCallingPoint(const char* newCp);

    /**
     * @brief Set the platform text to show on the right.
     * @param newPlat The platform string.
     */
    void setPlatform(const char* newPlat);

    /**
     * @brief Adjust the header's time display offset.
     * @param offset Minutes to offset relative to system time.
     * @designer_prop int timeOffset = 0 - Timezone offset in minutes.
     */
    void setTimeOffset(int offset);

    /**
     * @brief Toggle rendering the date underneath the time.
     * @param show True to render the date.
     * @designer_prop bool showDate = true - Toggle visibility of the current date.
     */
    void setShowDate(bool show);

    /**
     * @brief Ensure the text scroll animation restarts from the beginning.
     */
    void resetScroll();

    /**
     * @brief Get access to the inner clock widget.
     * @return Reference to the ClockWidget.
     */
    clockWidget& getClock();

    /**
     * @brief Update scrolling animation state.
     * @param currentMillis Current system uptime in milliseconds.
     */
    void tick(uint32_t currentMillis) override;

    /**
     * @brief Render the header bar to the display.
     * @param display The U8G2 display context.
     */
    void render(U8G2& display) override;

    /**
     * @brief Perform partial updates for title scrolling.
     * @param display The U8G2 display context.
     * @param currentMillis Current system uptime in milliseconds.
     */
    void renderAnimationUpdate(U8G2& display, uint32_t currentMillis) override;
};

#endif // HEADER_WIDGET_HPP
