/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/gfxUtilities/drawingPrimitives.hpp
 * Description: Contains stateless U8G2 utility wrappers for text layout, measurement, 
 *              and basic loading indicators.
 *
 * Exported Functions/Classes:
 * - blankArea: Clears a rectangular area on the display by drawing a black box.
 * - getStringWidth: Calculates the pixel width of a string using the current font.
 * - drawTruncatedText: Renders text left-aligned, truncating with ellipsis if too long.
 * - centreText: Renders text horizontally centered on the screen.
 */

#pragma once

#include <U8g2lib.h>
#include <Arduino.h>

#define SCREEN_WIDTH 256 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define DIMMED_BRIGHTNESS 1 // OLED display brightness level when in sleep/screensaver mode

extern const uint8_t NatRailSmall9[];
extern const uint8_t NatRailTall12[];
extern const uint8_t NatRailClockSmall7[];
extern const uint8_t NatRailClockLarge9[];
extern const uint8_t Underground10[];
extern const uint8_t UndergroundClock8[];

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


