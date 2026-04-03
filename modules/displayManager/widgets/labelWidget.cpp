/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/widgets/labelWidget.cpp
 * Description: Implementation of stateful text rendering with alignment.
 *
 * Exported Functions/Classes:
 * - labelWidget: [Class implementation]
 *   - setText(): Assigns the display string buffer.
 *   - setFont(): Assigns the typography for rendering.
 *   - setAlignment(): Configures horizontal justification.
 *   - render(): Paints the text to the display buffer.
 */

#include "labelWidget.hpp"
#include "drawingPrimitives.hpp"
#include <string.h>

/**
 * @brief Initialize the label widget with default alignment.
 * @param _x X coordinate.
 * @param _y Y coordinate (baseline).
 * @param _w Width (-1 for auto).
 * @param _h Height (-1 for auto).
 */
labelWidget::labelWidget(int _x, int _y, int _w, int _h) 
    : iGfxWidget(_x, _y, _w, _h), font(nullptr), alignment(0), isTruncated(false) {
    memset(textBuffer, 0, sizeof(textBuffer));
}

/**
 * @brief Set the label text.
 * @param text C-string source.
 */
void labelWidget::setText(const char* text) {
    if (text) {
        strncpy(textBuffer, text, sizeof(textBuffer) - 1);
        textBuffer[sizeof(textBuffer) - 1] = '\0';
    } else {
        textBuffer[0] = '\0';
    }
}

void labelWidget::setFont(const uint8_t* newFont) {
    font = newFont;
}

void labelWidget::setAlignment(int align) {
    alignment = align;
}

void labelWidget::setTruncated(bool truncated) {
    isTruncated = truncated;
}

void labelWidget::tick(uint32_t currentMillis) {
    // labelWidget currently utilizes static geometry with no ongoing animation mechanics
    (void)currentMillis;
}

void labelWidget::render(U8G2& display) {
    if (!isVisible || !font || textBuffer[0] == '\0') {
        return;
    }

    U8g2StateSaver stateSaver(display);
    display.setFont(font);
    
    // drawText natively handles mathematical alignment, width boundary truncation, 
    // and layout offsets transparently against the active U8G2 context.
    drawText(display, textBuffer, x, y, width, height, static_cast<TextAlign>(alignment), isTruncated);
}
