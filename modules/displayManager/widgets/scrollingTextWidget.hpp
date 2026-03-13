/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/widgets/scrollingTextWidget.hpp
 * Description: Generic UI component that handles horizontal marquee scrolling 
 *              logic for any arbitrary text.
 *
 * Exported Functions/Classes:
 * - scrollingTextWidget: Base graphics widget for scrolling strings.
 */

#ifndef SCROLLING_TEXT_WIDGET_HPP
#define SCROLLING_TEXT_WIDGET_HPP

#include "iGfxWidget.hpp"
#include <U8g2lib.h>

/**
 * @brief Base widget responsible purely for the mathematics and rendering 
 *        of scrolling text within a physical bounding box.
 */
class scrollingTextWidget : public iGfxWidget {
protected:
    char currentText[512];        // Buffer for the active scrolling string
    int scrollX;                  // Current pixel offset for the scroll
    int messageWidth;             // Total rendered width of the string in pixels
    uint32_t lastScrollMs;        // Timestamp of the last scroll tick
    bool needsScroll;             // True if text is wider than bounding box
    bool needsLayout;             // True if string/font changed and widths need recalc
    const uint8_t* font;          // Assigned pointer to U8g2 font array
    bool scrollFinished;          // Flag indicating the string has fully exited the left bound

public:
    /**
     * @brief Construct a new generic scrolling text widget.
     * @param _x X-coordinate of top-left corner.
     * @param _y Y-coordinate of top-left corner.
     * @param _w Width of bounding box. Use 0 for screen width.
     * @param _h Height of bounding box.
     * @param _font U8g2 font array to use for rendering.
     */
    scrollingTextWidget(int _x, int _y, int _w, int _h, const uint8_t* _font);
    virtual ~scrollingTextWidget() = default;

    /**
     * @brief Set the raw text to be scrolled. Calling this resets layout math.
     * @param newText Null-terminated string.
     */
    void setText(const char* newText);

    /**
     * @brief True when the text completes a full scroll loop off-screen.
     *        Useful for derived classes to trigger data loads.
     * @return bool
     */
    bool isScrollFinished() const { return scrollFinished; }

    /**
     * @brief Manually resets the scroll position to the right edge.
     */
    void resetScroll();

    /**
     * @brief Recalculate scrolling state based on time elapsed.
     * @param currentMillis Standard millis() reference clock.
     */
    virtual void tick(uint32_t currentMillis) override;

    /**
     * @brief Initial full-frame render of the text inside the bounding box.
     * @param display The hardware u8g2 instance.
     */
    virtual void render(U8G2& display) override;

    /**
     * @brief Optimized partial-screen refresh during animation loops.
     * @param display The hardware u8g2 instance.
     * @param currentMillis Standard millis() reference clock.
     */
    virtual void renderAnimationUpdate(U8G2& display, uint32_t currentMillis) override;
};

#endif // SCROLLING_TEXT_WIDGET_HPP
