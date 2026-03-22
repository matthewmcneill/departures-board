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
 * Description: Low-level graphics utility library providing stateless wrappers 
 *              for text layout, measurement, and geometric primitives. Enhances 
 *              the base U8G2 library with centering and truncation logic.
 *
 * Exported Functions/Classes:
 * - blankArea(): Clears a rectangular screen region.
 * - getStringWidth(): Pixel measurement for standard and PROGMEM strings.
 * - drawTruncatedText(): Left-aligned text with automatic ellipsis.
 * - centreText(): Horizontally centered text layout.
 * - drawBox(): Geometric rectangle primitive (Filled/Frame).
 * - drawLine(): Bresenham line primitive.
 * - drawCircle(): Geometric circle primitive (Filled/Frame).
 * - drawRoundedBox(): Rounded rectangle primitive (Filled/Frame).
 * - drawTriangle(): Geometric triangle primitive (Filled/Frame).
 */

#pragma once

#include <U8g2lib.h>
#include <Arduino.h>

#define SCREEN_WIDTH 256  // OLED display width in pixels
#define SCREEN_HEIGHT 64  // OLED display height in pixels
#define DIMMED_BRIGHTNESS 1 // Minimum hardware contrast for sleep/screensaver mode

extern const uint8_t NatRailSmall9[];      // 9px condensed font for NR data
extern const uint8_t NatRailTall12[];      // 12px condensed font for NR headings
extern const uint8_t NatRailClockSmall7[]; // 7px numeric font for small clocks
extern const uint8_t NatRailClockLarge9[]; // 9px numeric font for large clocks
extern const uint8_t Underground10[];      // 10px Johnston-style font for TfL
extern const uint8_t UndergroundClock8[];  // 8px Johnston-style font for TfL clocks

/**
 * @brief Clear a rectangular area on the OLED display by drawing a black box
 * @param x Top left X coordinate
 * @param y Top left Y coordinate
 * @param w Width of the area
 * @param h Height of the area
 */
void blankArea(U8G2& display, int x, int y, int w, int h);

/**
 * @brief Calculate the width in pixels of a string using the current font
 * @param message C-string to measure
 * @return Width in pixels
 */
int getStringWidth(U8G2& display, const char *message);

/**
 * @brief Calculate the width in pixels of a PROGMEM string using the current font
 * @param message Flash string to measure
 * @return Width in pixels
 */
int getStringWidth(U8G2& display, const __FlashStringHelper *message);

/**
 * @brief Draw text left-aligned, truncating with '...' if it exceeds screen width
 * @param message The text to display
 * @param line The Y coordinate for the baseline
 */
void drawTruncatedText(U8G2& display, const char *message, int line);

/**
 * @brief Draw text horizontally centered on the screen
 * @param message The text to display
 * @param line The Y coordinate for the baseline
 */
void centreText(U8G2& display, const char *message, int line);

/**
 * @brief Draw PROGMEM text horizontally centered on the screen
 * @param message The PROGMEM text to display
 * @param line The Y coordinate for the baseline
 */
void centreText(U8G2& display, const __FlashStringHelper *message, int line);

/**
 * @brief Draw a rectangle (box) on the OLED display
 * @param x Top left X coordinate
 * @param y Top left Y coordinate
 * @param w Width of the box
 * @param h Height of the box
 * @param isFilled true to fill the box, false for an outline only
 */
void drawBox(U8G2& display, int x, int y, int w, int h, bool isFilled = false);

/**
 * @brief Draw a straight line between two points
 * @param x0 Start X coordinate
 * @param y0 Start Y coordinate
 * @param x1 End X coordinate
 * @param y1 End Y coordinate
 */
void drawLine(U8G2& display, int x0, int y0, int x1, int y1);

/**
 * @brief Draw a circle on the OLED display
 * @param x Center X coordinate
 * @param y Center Y coordinate
 * @param r Radius of the circle
 * @param isFilled true to fill the circle, false for an outline only
 */
void drawCircle(U8G2& display, int x, int y, int r, bool isFilled = false);

/**
 * @brief Draw a rectangle with rounded corners on the OLED display
 * @param x Top left X coordinate
 * @param y Top left Y coordinate
 * @param w Width of the box
 * @param h Height of the box
 * @param r Radius of the rounded corners
 * @param isFilled true to fill the box, false for an outline only
 */
void drawRoundedBox(U8G2& display, int x, int y, int w, int h, int r, bool isFilled = false);

/**
 * @brief Draw a triangle on the OLED display
 * @param x0 First vertex X coordinate
 * @param y0 First vertex Y coordinate
 * @param x1 Second vertex X coordinate
 * @param y1 Second vertex Y coordinate
 * @param x2 Third vertex X coordinate
 * @param y2 Third vertex Y coordinate
 * @param isFilled true to fill the triangle, false for an outline only
 */
void drawTriangle(U8G2& display, int x0, int y0, int x1, int y1, int x2, int y2, bool isFilled = false);


