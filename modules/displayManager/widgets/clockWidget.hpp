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
 * Description: Renders the system time with a configurable font and blink.
 *
 * Exported Functions/Classes:
 * - clockWidget: [Class] Graphical widget for real-time display.
 *   - setFont(): Update the primary typography.
 *   - setBlink(): Toggle the 1Hz colon animation.
 *   - tick(): Logic update for blink timing.
 *   - render(): Primary drawing method.
 */
#ifndef CLOCK_WIDGET_HPP
#define CLOCK_WIDGET_HPP

#include "iGfxWidget.hpp"

class TimeManager;

class clockWidget : public iGfxWidget {
public:
    /**
     * @brief Formats supported by the clock widget.
     */
    enum class ClockFormat {
        HH_MM,    ///< Standard hour and minute display (e.g., 14:36)
        HH_MM_SS  ///< Full display including seconds (e.g., 14:36:50)
    };

private:
    bool showColon;
    bool oldColon;
    bool blinkEnabled;
    uint32_t lastBlinkMs;
    int lastMinute;
    int lastSecond;
    ClockFormat format;
    const uint8_t* font;
    const uint8_t* secondaryFont;
    int alignment; ///< 0=Left, 1=Center, 2=Right
    TimeManager* timeMgr; ///< Pointer to the injected TimeManager instance

public:
    /**
     * @brief Construct a new clock widget.
     * @param _timeMgr Pointer to the time manager dependency.
     * @param _x X position.
     * @param _y Y position.
     * @param _w Width (-1 for auto).
     * @param _h Height (-1 for auto).
     * @param _font Pointer to the primary u8g2 font to use.
     */
    clockWidget(TimeManager* _timeMgr, int _x, int _y, int _w = -1, int _h = -1, const uint8_t* _font = nullptr);

    /**
     * @brief Update the primary font used by the clock.
     * @param newFont Pointer to the new u8g2 font array.
     * @designer_prop font font = "Underground10" - The main typography for the clock.
     */
    void setFont(const uint8_t* newFont);

    /**
     * @brief Update the secondary font used for the seconds component.
     * @param newFont Pointer to the new u8g2 font array (or nullptr to use the primary font).
     * @designer_prop font secondaryFont = "" - Optional secondary font for the seconds digits.
     */
    void setSecondaryFont(const uint8_t* newFont);

    /**
     * @brief Set the format of the clock display.
     * @param newFormat The ClockFormat enum value.
     * @designer_prop enum format = "HH_MM"(HH_MM), "HH_MM_SS"(HH_MM_SS) - The time format.
     */
    void setFormat(ClockFormat newFormat);

    /**
     * @brief Toggle the blinking colon animation.
     * @param enable True to enable blinking, false for solid colon.
     * @designer_prop bool blinkEnabled = true - Toggle colon blinking animation.
     */
    void setBlink(bool enable) { blinkEnabled = enable; showColon = true; }
    
    /**
     * @brief Set the horizontal alignment strategy inside the widget width.
     * @param align 0=Left, 1=Center, 2=Right.
     * @designer_prop int align = 1 - Horizontal text alignment.
     */
    void setAlignment(int align) { alignment = align; }
    
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
