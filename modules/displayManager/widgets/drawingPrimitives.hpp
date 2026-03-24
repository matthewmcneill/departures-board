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
 * Description: Stateless U8G2 graphics utility wrappers.
 *
 * Exported Functions/Classes:
 * - drawText(): Draw aligned/truncated text within a bounding box.
 * - drawBox(): Draw filled or outlined rectangle.
 * - drawLine(): Draw straight line segment.
 * - drawCircle(): Draw disc or outline circle.
 * - drawRoundedBox(): Draw rounded rectangle.
 * - drawTriangle(): Draw filled or outlined triangle.
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
 * @brief Draw a rectangle (box or frame)
 * @param display Reference to U8g2 context.
 * @param x Top-left X.
 * @param y Top-left Y.
 * @param w Width.
 * @param h Height.
 * @param isFilled true for filled box, false for frame.
 */
void drawBox(U8G2& display, int x, int y, int w, int h, bool isFilled = true);

/**
 * @brief Draw a straight line
 * @param display Reference to U8g2 context.
 * @param x0 Start X.
 * @param y0 Start Y.
 * @param x1 End X.
 * @param y1 End Y.
 */
void drawLine(U8G2& display, int x0, int y0, int x1, int y1);

/**
 * @brief Draw a circle (disc or outline)
 * @param display Reference to U8g2 context.
 * @param x Center X.
 * @param y Center Y.
 * @param r Radius.
 * @param isFilled true for disc, false for circle.
 */
void drawCircle(U8G2& display, int x, int y, int r, bool isFilled = true);

/**
 * @brief Draw a rounded rectangle
 * @param display Reference to U8g2 context.
 * @param x Top-left X.
 * @param y Top-left Y.
 * @param w Width.
 * @param h Height.
 * @param r Boundary radius.
 * @param isFilled true for filled, false for frame.
 */
void drawRoundedBox(U8G2& display, int x, int y, int w, int h, int r, bool isFilled = true);

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
