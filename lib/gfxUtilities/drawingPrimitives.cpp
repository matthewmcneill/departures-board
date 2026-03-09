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
void blankArea(int x, int y, int w, int h) {
  u8g2.setDrawColor(0);
  u8g2.drawBox(x,y,w,h);
  u8g2.setDrawColor(1);
}

/**
 * @brief Calculate the width in pixels of a string using the current font
 * @param message C-string to measure
 * @return Width in pixels
 */
int getStringWidth(const char *message) {
  return u8g2.getStrWidth(message);
}

/**
 * @brief Calculate the width in pixels of a PROGMEM string using the current font
 * @param message Flash string to measure
 * @return Width in pixels
 */
int getStringWidth(const __FlashStringHelper *message) {
  char buf[256];
  strcpy_P(buf, (const char*)message);
  return u8g2.getStrWidth(buf);
}

/**
 * @brief Draw text left-aligned, truncating with '...' if it exceeds screen width
 * @param message The text to display
 * @param line The Y coordinate for the baseline
 */
void drawTruncatedText(const char *message, int line) {
  int w = getStringWidth(message);
  char buf[256];
  strcpy(buf,message);
  if (w > 256) {
    while (getStringWidth(buf) > 249) buf[strlen(buf)-1]='\0';
    strcat(buf,"...");
  }
  u8g2.drawStr(0, line, buf);
}

/**
 * @brief Draw text horizontally centered on the screen
 * @param message The text to display
 * @param line The Y coordinate for the baseline
 */
void centreText(const char *message, int line) {
  int w = getStringWidth(message);
  if (w > 256) drawTruncatedText(message, line);
  else {
    int start = (256-w)/2;
    u8g2.drawStr(start, line, message);
  }
}

/**
 * @brief Draw PROGMEM text horizontally centered on the screen
 * @param message The PROGMEM text to display
 * @param line The Y coordinate for the baseline
 */
void centreText(const __FlashStringHelper *message, int line) {
  char buf[256];
  strcpy_P(buf, (const char*)message);
  centreText(buf, line);
}

/**
 * @brief Draw a progress bar at the bottom of the screen
 * @param percent Progress amount (0-100)
 */
void drawProgressBar(int percent) {
  int fill = (percent*256)/100;
  bool toggle = false;
  u8g2.setDrawColor(0);
  u8g2.drawBox(0,30,256,2);
  u8g2.setDrawColor(1);

  for (int x=0; x<fill; x++) {
    if (toggle) {
      u8g2.drawHVLine(x,30,1,1);
      u8g2.drawHVLine(x,31,1,0);
    } else {
      u8g2.drawHVLine(x,30,1,0);     
      u8g2.drawHVLine(x,31,1,1);
    }
    toggle=!toggle;
  }
}

/**
 * @brief Draw centered status text and update the progress bar
 * @param text Status description
 * @param percent Progress amount (0-100)
 */
void progressBar(const char *text, int percent) {
  blankArea(0, 15, 256, 17);
  centreText(text, 26);
  drawProgressBar(percent);
  u8g2.sendBuffer();
}

/**
 * @brief Draw centered PROGMEM status text and update the progress bar
 * @param text PROGMEM Status description
 * @param percent Progress amount (0-100)
 */
void progressBar(const __FlashStringHelper *text, int percent) {
  char buf[256];
  strcpy_P(buf, (const char*)text);
  progressBar(buf, percent);
}
