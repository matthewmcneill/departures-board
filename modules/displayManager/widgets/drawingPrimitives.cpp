/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/widgets/drawingPrimitives.cpp
 * Description: Implementation of stateless U8G2 graphics utility wrappers.
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
 * @brief Draw text with alignment and optional truncation within a bounding box.
 * Optimized for performance with a fast-path for the most common use cases.
 */
void drawText(U8G2& display, const char *message, int x, int y, int w, int h, TextAlign align, bool truncate, const uint8_t* font) {
  if (font) display.setFont(font);
  
  // --- FAST-PATH: Standard left-aligned, non-truncated text with no bounding box ---
  // This bypasses all measurement and clipping logic (O(1) overhead).
  if (align == TextAlign::LEFT && !truncate && w == -1 && h == -1) {
    display.drawStr(x, y, message);
    return;
  }

  // --- FULL-PATH: Required for alignment, truncation, or explicit clipping ---
  
  // 1. Resolve bounding box defaults
  if (w == -1) w = 256 - x; 
  if (h == -1) h = display.getMaxCharHeight();
  
  int drawX = x;
  int textW = 0;

  // 2. Defer measurement until absolutely needed
  if (align != TextAlign::LEFT || truncate) {
    textW = display.getStrWidth(message);
    if (align == TextAlign::CENTER) {
      drawX += (w - textW) / 2;
    } else if (align == TextAlign::RIGHT) {
      drawX += (w - textW);
    }
  }

  // 3. Clipping and Rendering
  display.setClipWindow(x, y, x + w, y + h);

  if (truncate && textW > w) {
    // Truncation requires a stack buffer
    char buf[256];
    strlcpy(buf, message, sizeof(buf));
    int ellipsisW = display.getStrWidth("...");
    
    // Optimization: Rough-cut based on average character width to avoid O(N) getStrWidth calls
    int len = strlen(buf);
    if (len > 0) {
      int roughLen = (w - ellipsisW) / (textW / len);
      if (roughLen < len) buf[roughLen] = '\0';
    }

    // Fine-tune for pixel perfection
    while (strlen(buf) > 0 && display.getStrWidth(buf) > (w - ellipsisW)) {
        buf[strlen(buf)-1] = '\0';
    }
    strcat(buf, "...");
    display.drawStr(drawX, y, buf);
  } else {
    display.drawStr(drawX, y, message);
  }

  display.setMaxClipWindow();
}

/**
 * @brief Draw PROGMEM text with alignment and optional truncation within a bounding box.
 */
void drawText(U8G2& display, const __FlashStringHelper *message, int x, int y, int w, int h, TextAlign align, bool truncate, const uint8_t* font) {
  char buf[256];
  strcpy_P(buf, (const char*)message);
  drawText(display, buf, x, y, w, h, align, truncate, font);
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
    // Use hardware filled triangle routine
    display.drawTriangle(x0, y0, x1, y1, x2, y2);
  } else {
    // Manually trace the vertices for an outline (not present in base U8G2)
    display.drawLine(x0, y0, x1, y1);
    display.drawLine(x1, y1, x2, y2);
    display.drawLine(x2, y2, x0, y0);
  }
}


