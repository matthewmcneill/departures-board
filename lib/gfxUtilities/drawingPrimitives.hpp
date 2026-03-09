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
 */

#pragma once

#include <U8g2lib.h>
#include <Arduino.h>

#define SCREEN_WIDTH 256 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define DIMMED_BRIGHTNESS 1 // OLED display brightness level when in sleep/screensaver mode

extern U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI u8g2;

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
void blankArea(int x, int y, int w, int h);

/**
 * @brief Calculate the width in pixels of a string using the current font
 * @param message C-string to measure
 * @return Width in pixels
 */
int getStringWidth(const char *message);

/**
 * @brief Calculate the width in pixels of a PROGMEM string using the current font
 * @param message Flash string to measure
 * @return Width in pixels
 */
int getStringWidth(const __FlashStringHelper *message);

/**
 * @brief Draw text left-aligned, truncating with '...' if it exceeds screen width
 * @param message The text to display
 * @param line The Y coordinate for the baseline
 */
void drawTruncatedText(const char *message, int line);

/**
 * @brief Draw text horizontally centered on the screen
 * @param message The text to display
 * @param line The Y coordinate for the baseline
 */
void centreText(const char *message, int line);

/**
 * @brief Draw PROGMEM text horizontally centered on the screen
 * @param message The PROGMEM text to display
 * @param line The Y coordinate for the baseline
 */
void centreText(const __FlashStringHelper *message, int line);

/**
 * @brief Draw a progress bar at the bottom of the screen
 * @param percent Progress amount (0-100)
 */
void drawProgressBar(int percent);

/**
 * @brief Draw centered status text and update the progress bar
 * @param text Status description
 * @param percent Progress amount (0-100)
 */
void progressBar(const char *text, int percent);

/**
 * @brief Draw centered PROGMEM status text and update the progress bar
 * @param text PROGMEM Status description
 * @param percent Progress amount (0-100)
 */
void progressBar(const __FlashStringHelper *text, int percent);
