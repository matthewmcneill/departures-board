/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/widgets/clockWidget.hpp
 * Description: Renders the system clock with a configurable font and blinking colon.
 *
 * Exported Functions/Classes:
 * - clockWidget: Graphical widget that draws the time.
 *   - setFont(): Change the active font.
 *   - setBlink(): Toggle the colon blinking effect.
 *   - tick(): Update the internal clock state.
 *   - render(): Draw the clock to the display buffer.
 *   - renderAnimationUpdate(): Handle blink animation frames.
 */
#ifndef CLOCK_WIDGET_HPP
#define CLOCK_WIDGET_HPP

#include "iGfxWidget.hpp"

class TimeManager;

class clockWidget : public iGfxWidget {
private:
    bool showColon;
    bool blinkEnabled;
    uint32_t lastBlinkMs;
    int lastMinute;
    const uint8_t* font;
    TimeManager* timeMgr; ///< Pointer to the injected TimeManager instance

public:
    /**
     * @brief Construct a new clock widget.
     * @param _x X position.
     * @param _y Y position.
     * @param _w Width (-1 for auto).
     * @param _h Height (-1 for auto).
     * @param _font Pointer to the u8g2 font to use.
     */
    clockWidget(TimeManager* _timeMgr, int _x, int _y, int _w = -1, int _h = -1, const uint8_t* _font = nullptr);

    /**
     * @brief Update the font used by the clock.
     * @param newFont Pointer to the new u8g2 font array.
     */
    void setFont(const uint8_t* newFont);

    /**
     * @brief Toggle the blinking colon animation.
     * @param enable True to enable blinking, false for solid colon.
     */
    void setBlink(bool enable) { blinkEnabled = enable; showColon = true; }
    
    /**
     * @brief Update the clock's internal time and animation state.
     * @param currentMillis Current system uptime in milliseconds.
     */
    void tick(uint32_t currentMillis) override;

    /**
     * @brief Render the clock to the provided display instance.
     * @param display The U8G2 display context.
     */
    void render(U8G2& display) override;

    /**
     * @brief Perform partial updates for the blinking animation if enabled.
     * @param display The U8G2 display context.
     * @param currentMillis Current system uptime in milliseconds.
     */
    void renderAnimationUpdate(U8G2& display, uint32_t currentMillis) override;
};

#endif // CLOCK_WIDGET_HPP
