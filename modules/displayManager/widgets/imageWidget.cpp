/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/widgets/imageWidget.cpp
 * Description: Implementation of static XBM bitmap rendering.
 *
 * Exported Functions/Classes:
 * - imageWidget: [Class implementation]
 *   - render(): Paints the XBM bits to the display buffer.
 */

#include <widgets/imageWidget.hpp>

/**
 * @brief Initialize the static image widget.
 * @param _x X coordinate.
 * @param _y Y coordinate.
 * @param _w width.
 * @param _h height.
 * @param bits Pointer to XBM bits (expected in PROGMEM).
 */
imageWidget::imageWidget(int _x, int _y, int _w, int _h, const unsigned char* bits)
    : iGfxWidget(_x, _y, _w, _h), imageBits(bits) {
}

/**
 * @brief No-op. Static images do not need state updates.
 * @param currentMillis Current system time in milliseconds.
 */
void imageWidget::tick(uint32_t currentMillis) {
    // Static widgets do not update state
}

/**
 * @brief Renders the image to the display buffer if visible.
 * @param display Reference to the global U8g2 instance.
 */
void imageWidget::render(U8G2& display) {
    if (isVisible && imageBits != nullptr) {
        display.drawXBM(x, y, width, height, imageBits);
    }
}
