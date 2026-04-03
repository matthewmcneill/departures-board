/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/widgets/progressBarWidget.hpp
 * Description: Graphical progress indicator with message support.
 *
 * Exported Functions/Classes:
 * - progressBarWidget: [Class] Graphics widget for showing task progression.
 *   - setMessage(): Updates the primary label center-aligned above the bar.
 *   - setPercent(): Primary entry for linear progress updates.
 *   - tick(): Logic for smooth frame-interpolated animation.
 *   - render(): Initial full-frame drawing.
 */

#ifndef PROGRESS_BAR_WIDGET_HPP
#define PROGRESS_BAR_WIDGET_HPP

#include <Arduino.h>
#include "iGfxWidget.hpp"

class progressBarWidget : public iGfxWidget {
private:
    char message[256];
    int currentPercent;
    int targetPercent;
    int startPercent;
    uint32_t animStartTime;
    uint32_t animDurationMs;
    
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
     * @designer_prop bool showPercentText = true - Render the numeric percentage.
     */
    void setShowPercentText(bool show);

    /**
     * @brief Sets the progress value (0-100) and optionally animates to it.
     * @param percent Target progress amount.
     * @param durationMs Animation duration in milliseconds. 0 for instant snap.
     */
    void setPercent(int percent, uint32_t durationMs = 0);

    void tick(uint32_t currentMillis) override;
    void render(U8G2& display) override;
    void renderAnimationUpdate(U8G2& display, uint32_t currentMillis) override;
};

#endif // PROGRESS_BAR_WIDGET_HPP
