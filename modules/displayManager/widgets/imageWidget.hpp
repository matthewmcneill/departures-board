/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/widgets/imageWidget.hpp
 * Description: Renders static XBM-encoded monochrome bitmap images (branding/icons). 
 *              Optimized for PROGMEM storage to minimize RAM footprint.
 *
 * Exported Functions/Classes:
 * - imageWidget: UI widget for static bitmap display.
 *   - tick(): No-op lifecycle hook.
 *   - render(): Primary drawing method.
 */

#ifndef IMAGE_WIDGET_HPP
#define IMAGE_WIDGET_HPP

#include "iGfxWidget.hpp"

/**
 * @brief Widget to display a static XBM format image.
 */
class imageWidget : public iGfxWidget {
private:
    const unsigned char* imageBits;

public:
    /**
     * @brief Constructor for the Image widget.
     * @param _x X coordinate (top-left).
     * @param _y Y coordinate (top-left).
     * @param _w Pixel width of the image.
     * @param _h Pixel height of the image.
     * @param bits Pointer to the XBM PROGMEM byte array.
     */
    imageWidget(int _x, int _y, int _w, int _h, const unsigned char* bits);

    /**
     * @brief No-op. Static images do not need state updates.
     * @param currentMillis Current system time in milliseconds.
     */
    void tick(uint32_t currentMillis) override;

    /**
     * @brief Renders the image to the display buffer if visible.
     * @param display Reference to the global U8g2 instance.
     */
    void render(U8G2& display) override;
};

#endif // IMAGE_WIDGET_HPP
