/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/widgets/drawingPrimitives.hpp
 * Description: Stateless U8G2 graphics utility wrappers for common drawing tasks.
 *
 * Exported Functions/Classes:
 * - TextAlign: [Enum] Text justification options (LEFT, CENTER, RIGHT).
 * - U8g2StateSaver: [Class] RAII scope guard to preserve U8G2 display properties.
 * - blankArea(): Clears a rectangular region.
 * - getStringWidth(): Measures string width in pixels.
 * - drawText(): Aligned text rendering with optional truncation.
 * - drawTriangle(): Draws filled or outlined triangles.
 */

#ifndef DRAWING_PRIMITIVES_HPP
#define DRAWING_PRIMITIVES_HPP

#include <U8g2lib.h>
#include <Arduino.h>

/**
 * @brief Text alignment options for consolidated drawing primitives.
 */
enum class TextAlign : uint8_t {
    LEFT = 0,
    CENTER = 1,
    RIGHT = 2
};

/**
 * @brief RAII scope guard to preserve and restore U8G2 display properties.
 * Tracks clipping windows, colors, and fonts natively without heap allocation.
 */
class U8g2StateSaver {
private:
    U8G2& display;               // Reference to the display context
    const uint8_t* oldFont;      // Cached font pointer
    uint8_t oldDrawColor;        // Cached draw color
    u8g2_uint_t oldClipX0;       // Cached top-left X clip bound
    u8g2_uint_t oldClipY0;       // Cached top-left Y clip bound
    u8g2_uint_t oldClipX1;       // Cached bottom-right X clip bound
    u8g2_uint_t oldClipY1;       // Cached bottom-right Y clip bound

public:
    /**
     * @brief Constructs the saver, caching current state.
     * @param _display active U8G2 instance.
     */
    U8g2StateSaver(U8G2& _display);

    /**
     * @brief Destructor automatically reverts display state.
     */
    ~U8g2StateSaver();
};

/**
 * @brief Clear a rectangular area on the OLED display
 * @param display Reference to U8g2 context.
 * @param x Top-left X.
 * @param y Top-left Y.
 * @param w Width.
 * @param h Height.
 */
void blankArea(U8G2& display, int x, int y, int w, int h);

/**
 * @brief Calculate the width in pixels of a string
 * @param display Reference to U8g2 context.
 * @param message Text to measure.
 * @return Width in pixels.
 */
int getStringWidth(U8G2& display, const char *message);
int getStringWidth(U8G2& display, const __FlashStringHelper *message);

/**
 * @brief Draw text with alignment and optional truncation within a bounding box.
 * @param display Reference to U8g2 context.
 * @param message Text to display.
 * @param x Top-left X coordinate of the text area.
 * @param y Top-left Y coordinate of the text area.
 * @param w Width of the area (-1 for screen width).
 * @param h Height of the area (-1 for font height).
 * @param align Alignment within the width (LEFT, CENTER, RIGHT).
 * @param truncate If true, appends '...' if text exceeds width.
 * @param font Optional font override. If nullptr, current font is used.
 */
void drawText(U8G2& display, const char *message, int x, int y, int w = -1, int h = -1, TextAlign align = TextAlign::LEFT, bool truncate = false, const uint8_t* font = nullptr);
void drawText(U8G2& display, const __FlashStringHelper *message, int x, int y, int w = -1, int h = -1, TextAlign align = TextAlign::LEFT, bool truncate = false, const uint8_t* font = nullptr);



/**
 * @brief Draw a triangle
 * @param display Reference to U8g2 context.
 * @param x0 Point 0 X.
 * @param y0 Point 0 Y.
 * @param x1 Point 1 X.
 * @param y1 Point 1 Y.
 * @param x2 Point 2 X.
 * @param y2 Point 2 Y.
 * @param isFilled true for filled, false for outline.
 */
void drawTriangle(U8G2& display, int x0, int y0, int x1, int y1, int x2, int y2, bool isFilled = true);

#endif // DRAWING_PRIMITIVES_HPP
