/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
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

/**
 * @brief Draw a rectangle (box) on the OLED display
 * @param x Top left X coordinate
 * @param y Top left Y coordinate
 * @param w Width of the box
 * @param h Height of the box
 * @param isFilled true to fill the box, false for an outline only
 */
void drawBox(U8G2& display, int x, int y, int w, int h, bool isFilled) {
  if (isFilled) display.drawBox(x, y, w, h);
  else display.drawFrame(x, y, w, h);
}

/**
 * @brief Draw a straight line between two points
 * @param x0 Start X coordinate
 * @param y0 Start Y coordinate
 * @param x1 End X coordinate
 * @param y1 End Y coordinate
 */
void drawLine(U8G2& display, int x0, int y0, int x1, int y1) {
  display.drawLine(x0, y0, x1, y1);
}

/**
 * @brief Draw a circle on the OLED display
 * @param x Center X coordinate
 * @param y Center Y coordinate
 * @param r Radius of the circle
 * @param isFilled true to fill the circle, false for an outline only
 */
void drawCircle(U8G2& display, int x, int y, int r, bool isFilled) {
  if (isFilled) display.drawDisc(x, y, r);
  else display.drawCircle(x, y, r);
}

/**
 * @brief Draw a rectangle with rounded corners on the OLED display
 * @param x Top left X coordinate
 * @param y Top left Y coordinate
 * @param w Width of the box
 * @param h Height of the box
 * @param r Radius of the rounded corners
 * @param isFilled true to fill the box, false for an outline only
 */
void drawRoundedBox(U8G2& display, int x, int y, int w, int h, int r, bool isFilled) {
  if (isFilled) display.drawRBox(x, y, w, h, r);
  else display.drawRFrame(x, y, w, h, r);
}

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
void drawTriangle(U8G2& display, int x0, int y0, int x1, int y1, int x2, int y2, bool isFilled) {
  if (isFilled) {
    display.drawTriangle(x0, y0, x1, y1, x2, y2);
  } else {
    display.drawLine(x0, y0, x1, y1);
    display.drawLine(x1, y1, x2, y2);
    display.drawLine(x2, y2, x0, y0);
  }
}


