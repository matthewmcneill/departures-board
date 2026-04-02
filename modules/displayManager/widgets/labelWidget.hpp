/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/widgets/labelWidget.hpp
 * Description: Stateful text component with alignment and truncation.
 *
 * Exported Functions/Classes:
 * - labelWidget: [Class] Graphics widget for stateful text notifications.
 *   - setText(): Assigns the display string buffer.
 *   - setFont(): Assigns the typography for rendering.
 *   - setAlignment(): Configures horizontal justification.
 *   - render(): Paints the text to the display buffer.
 */
#ifndef LABEL_WIDGET_HPP
#define LABEL_WIDGET_HPP

#include "iGfxWidget.hpp"

/**
 * @brief A stateful text widget capable of rendering aligned and truncated text 
 *        inside a specific bounding box. Exposes designer properties for automated 
 *        UI generation.
 */
class labelWidget : public iGfxWidget {
private:
    char textBuffer[128];
    const uint8_t* font;
    int alignment; // 0=Left, 1=Center, 2=Right
    bool isTruncated;

public:
    /**
     * @brief Construct a new label widget.
     * @param _x X coordinate on the OLED.
     * @param _y Y coordinate (baseline) on the OLED.
     * @param _w Width of the bounding box (-1 for unconstrained).
     * @param _h Height of the bounding box (-1 for unconstrained).
     */
    labelWidget(int _x, int _y, int _w = -1, int _h = -1);

    /**
     * @brief Set the text payload for the label.
     * @param text The new string to display. Safely bounds to 127 characters.
     */
    void setText(const char* text);

    /**
     * @brief Apply a new font typography to the label.
     * @param newFont Pointer to the U8G2 font array.
     * @designer_prop font font = "Underground10" - The typography for the text.
     */
    void setFont(const uint8_t* newFont);

    /**
     * @brief Set the horizontal alignment strategy inside the widget width.
     * @param align 0=Left, 1=Center, 2=Right.
     * @designer_prop int align = 0 - Horizontal text alignment.
     */
    void setAlignment(int align);

    /**
     * @brief Enable or disable automated string truncation if it overshoots the width.
     * @param truncated True to append sequence '...' if the text exceeds layout boundaries.
     * @designer_prop bool isTruncated = false - Append ellipsis if text exceeds widget width.
     */
    void setTruncated(bool truncated);

    /**
     * @brief Standard logic update loop logic. Called once per cycle by DisplayManager.
     * @param currentMillis Current system uptime in milliseconds.
     */
    void tick(uint32_t currentMillis) override;

    /**
     * @brief Renders the textual data into the active display buffer.
     * @param display The U8G2 display driver reference.
     */
    void render(U8G2& display) override;
};

#endif // LABEL_WIDGET_HPP
