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
 *
 * Exported Functions/Classes:
 * - TextAlign: [Enum] Text justification options (LEFT, CENTER, RIGHT).
 * - U8g2StateSaver: [Class implementation] RAII display state management.
 * - blankArea(): Region clearing utility.
 * - getStringWidth(): Character width measurement.
 * - drawText(): Multi-aligned text rendering with truncation logic.
 * - drawTriangle(): Geometric primitive drawing.
 */

#include "drawingPrimitives.hpp"

// --- U8g2StateSaver Implementation ---

/**
 * @brief Constructs the state saver.
 * Caches the current font, color, and clipping window from the u8g2_t struct.
 * @param _display Reference to the active U8G2 instance.
 */
U8g2StateSaver::U8g2StateSaver(U8G2& _display) : display(_display) {
    u8g2_t *u8g2 = display.getU8g2();
    oldFont = u8g2->font;
    oldDrawColor = u8g2->draw_color;
    oldClipX0 = u8g2->clip_x0;
    oldClipY0 = u8g2->clip_y0;
    oldClipX1 = u8g2->clip_x1;
    oldClipY1 = u8g2->clip_y1;
}

/**
 * @brief Revert the display state to the cached values.
 */
U8g2StateSaver::~U8g2StateSaver() {
    display.setFont(oldFont);
    display.setDrawColor(oldDrawColor);
    display.setClipWindow(oldClipX0, oldClipY0, oldClipX1, oldClipY1);
}

/**
 * @brief Clear a rectangular area on the OLED display by drawing a black box
 * @param x Top left X coordinate
 * @param y Top left Y coordinate
 * @param w Width of the area
 * @param h Height of the area
 */
void blankArea(U8G2& display, int x, int y, int w, int h) {
  U8g2StateSaver saver(display);
  display.setDrawColor(0);
  display.drawBox(x,y,w,h);
}

/**
 * @brief Calculate the width in pixels of a string.
 * @param display U8g2 reference.
 * @param message Null-terminated string.
 * @return Width in pixels.
 */
int getStringWidth(U8G2& display, const char *message) {
  return display.getStrWidth(message);
}

/**
 * @brief Calculate the width in pixels of a Flash string.
 * @param display U8g2 reference.
 * @param message Flash string pointer.
 * @return Width in pixels.
 */
int getStringWidth(U8G2& display, const __FlashStringHelper *message) {
  char buf[256];
  strcpy_P(buf, (const char*)message);
  return display.getStrWidth(buf);
}

/**
 * @brief Core text rendering utility with alignment and optional truncation.
 * 
 * Optimized with a fast-path for standard left-aligned text to bypass 
 * measurement and clipping logic.
 * 
 * @param display U8g2 reference.
 * @param message String to draw.
 * @param x Left edge.
 * @param y Top edge.
 * @param w Width (-1 for screen).
 * @param h Height (-1 for char height).
 * @param align Justification.
 * @param truncate Append '...' if too wide.
 * @param font Optional font override.
 */
void drawText(U8G2& display, const char *message, int x, int y, int w, int h, TextAlign align, bool truncate, const uint8_t* font) {
  U8g2StateSaver saver(display);

  if (font) display.setFont(font);
  
  // --- FAST-PATH: Standard left-aligned, non-truncated text with no bounding box ---
  // This bypasses all measurement and clipping logic (O(1) overhead).
  if (align == TextAlign::LEFT && !truncate && w == -1 && h == -1) {
    display.drawStr(x, y, message);
    return; // State safely restored by RAII destructor
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
      drawX += (w - textW) - 1; // Shift left by 1 pixel to prevent clipping overhang
    }
  }

  // 3. Clipping and Rendering
  u8g2_t *u8g2_struct = display.getU8g2();
  
  // Intersect our desired clip with the existing clip
  u8g2_uint_t new_x0 = (x > u8g2_struct->clip_x0) ? x : u8g2_struct->clip_x0;
  u8g2_uint_t new_y0 = (y > u8g2_struct->clip_y0) ? y : u8g2_struct->clip_y0;
  u8g2_uint_t new_x1 = ((x + w - 1) < u8g2_struct->clip_x1) ? (x + w - 1) : u8g2_struct->clip_x1;
  u8g2_uint_t new_y1 = ((y + h - 1) < u8g2_struct->clip_y1) ? (y + h - 1) : u8g2_struct->clip_y1;

  // If intersection is valid, apply it. Otherwise, squash the drawing region to nothing.
  if (new_x0 <= new_x1 && new_y0 <= new_y1) {
      display.setClipWindow(new_x0, new_y0, new_x1, new_y1);
  } else {
      display.setClipWindow(0, 0, 0, 0);
  }

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
 * @brief Draw a triangle geometric primitive.
 * @param display U8g2 reference.
 * @param x0 Vertex 0 X.
 * @param y0 Vertex 0 Y.
 * @param x1 Vertex 1 X.
 * @param y1 Vertex 1 Y.
 * @param x2 Vertex 2 X.
 * @param y2 Vertex 2 Y.
 * @param isFilled true for solid, false for wireframe.
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


