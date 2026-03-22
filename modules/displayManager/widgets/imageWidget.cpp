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
 * Description: Implementation of XBM bitmap rendering.
 */

#include <widgets/imageWidget.hpp>

/**
 * @brief Constructor for the Image widget.
 * @param _x X coordinate (top-left).
 * @param _y Y coordinate (top-left).
 * @param _w Pixel width of the image.
 * @param _h Pixel height of the image.
 * @param bits Pointer to the XBM PROGMEM byte array.
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
