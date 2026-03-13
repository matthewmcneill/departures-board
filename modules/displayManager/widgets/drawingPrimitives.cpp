/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/gfxUtilities/drawingPrimitives.cpp
 * Description: Implementation of stateless U8G2 utility wrappers.
 */

#include "drawingPrimitives.hpp"

/**
 * @brief Clear a rectangular area on the OLED display by drawing a black box
 * @param x Top left X coordinate
 * @param y Top left Y coordinate
 * @param w Width of the area
 * @param h Height of the area
 */
void blankArea(U8G2& display, int x, int y, int w, int h) {
  display.setDrawColor(0);
  display.drawBox(x,y,w,h);
  display.setDrawColor(1);
}

/**
 * @brief Calculate the width in pixels of a string using the current font
 * @param message C-string to measure
 * @return Width in pixels
 */
int getStringWidth(U8G2& display, const char *message) {
  return display.getStrWidth(message);
}

/**
 * @brief Calculate the width in pixels of a PROGMEM string using the current font
 * @param message Flash string to measure
 * @return Width in pixels
 */
int getStringWidth(U8G2& display, const __FlashStringHelper *message) {
  char buf[256];
  strcpy_P(buf, (const char*)message);
  return display.getStrWidth(buf);
}

/**
 * @brief Draw text left-aligned, truncating with '...' if it exceeds screen width
 * @param message The text to display
 * @param line The Y coordinate for the baseline
 */
void drawTruncatedText(U8G2& display, const char *message, int line) {
  int w = getStringWidth(display, message);
  char buf[256];
  strcpy(buf,message);
  if (w > 256) {
    while (getStringWidth(display, buf) > 249) buf[strlen(buf)-1]='\0';
    strcat(buf,"...");
  }
  display.drawStr(0, line, buf);
}

/**
 * @brief Draw text horizontally centered on the screen
 * @param message The text to display
 * @param line The Y coordinate for the baseline
 */
void centreText(U8G2& display, const char *message, int line) {
  int w = getStringWidth(display, message);
  if (w > 256) drawTruncatedText(display, message, line);
  else {
    int start = (256-w)/2;
    display.drawStr(start, line, message);
  }
}

/**
 * @brief Draw PROGMEM text horizontally centered on the screen
 * @param message The PROGMEM text to display
 * @param line The Y coordinate for the baseline
 */
void centreText(U8G2& display, const __FlashStringHelper *message, int line) {
  char buf[256];
  strcpy_P(buf, (const char*)message);
  centreText(display, buf, line);
}


