/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/gfxUtilities/progressBarWidget.hpp
 * Description: Renders a progress bar with a message and a percentage.
 */

#ifndef PROGRESS_BAR_WIDGET_HPP
#define PROGRESS_BAR_WIDGET_HPP

#include "iGfxWidget.hpp"

class progressBarWidget : public iGfxWidget {
private:
    char message[256];
    int currentPercent;
    
    // State tracking for deduplication
    int oldPercent;
    bool textChanged;
    bool showPercentText;
    
    const uint8_t* font;

public:
    /**
     * @brief Construct a new progress bar widget.
     * @param _x X position.
     * @param _y Y position.
     * @param _w Width (-1 for full screen).
     * @param _h Height (-1 for auto height).
     * @param _font Pointer to the u8g2 font to use.
     */
    progressBarWidget(int _x, int _y, int _w = -1, int _h = -1, const uint8_t* _font = nullptr);

    /**
     * @brief Sets the text message.
     * @param newMessage The text to display above the bar.
     */
    void setMessage(const char* newMessage);
    
    /**
     * @brief Sets the text message from PROGMEM.
     * @param newMessage Flash string message.
     */
    void setMessage(const __FlashStringHelper* newMessage);

    /**
     * @brief Toggles whether the integer percentage value is rendered.
     * @param show True to render the percentage string (e.g. "45%").
     */
    void setShowPercentText(bool show);

    /**
     * @brief Sets the progress value (0-100).
     * @param percent Progress amount.
     */
    void setPercent(int percent);

    void tick(uint32_t currentMillis) override;
    void render(U8G2& display) override;
    void renderAnimationUpdate(U8G2& display, uint32_t currentMillis) override;
};

#endif // PROGRESS_BAR_WIDGET_HPP
