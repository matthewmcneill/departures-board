/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons
 * Attribution-NonCommercial-ShareAlike 4.0 International. To view a copy of
 * this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/widgets/trainFormationWidget.cpp
 * Description: Widget for rendering train formations visually with a
 * state-machine animation loop.
 *
 * Exported Functions/Classes:
 * - trainFormationWidget: Widget for drawing dynamic train carriage layouts natively
 *   - setFormationData(): Bound the internal pointer reference
 *   - draw(): The main rendering routine
 *   - tick(): Core animation state machine loop
 */

#include "trainFormationWidget.hpp"
#include "drawingPrimitives.hpp"
#include <string.h>

trainFormationWidget::trainFormationWidget(int _x, int _y, int _w, int _h)
    : iGfxWidget(_x, _y, _w, _h) {}

void trainFormationWidget::setFormationData(NrCoachFormation *data,
                                            uint8_t count) {
  formations = data;
  numCoaches = count;
}

void trainFormationWidget::tick(uint32_t ms) {
  if (numCoaches == 0 || !formations)
    return;

  int noseConeWidth = 5;
  int availableWidth = width - noseConeWidth;
  int carriageWidth = availableWidth / numCoaches;
  bool isMarquee = false;

  if (carriageWidth > MAX_COACH_WIDTH) {
    carriageWidth = MAX_COACH_WIDTH;
  } else if (carriageWidth < MIN_COACH_WIDTH) {
    carriageWidth = MIN_COACH_WIDTH;
    isMarquee = true;
  }

  int totalTrainWidth = noseConeWidth + (numCoaches * carriageWidth);

  if (isMarquee) {
    // Marquee Mode
    if (ms - lastAnimationMs >= 50) { // Scroll speed
      scrollOffset -= 1;
      int maxScroll = totalTrainWidth;
      if (scrollOffset < -maxScroll) {
        // Done panning across widget
        scrollOffset = width;
        int nextMode = (static_cast<int>(currentMode) + 1) % 3;
        currentMode = static_cast<Mode>(nextMode);
      }
      lastAnimationMs = ms;
    }
  } else {
    // Static Mode
    scrollOffset = 0;
    if (ms - lastAnimationMs >= intervalMs) {
      int nextMode = (static_cast<int>(currentMode) + 1) % 3;
      currentMode = static_cast<Mode>(nextMode);
      lastAnimationMs = ms;
    }
  }
}

void trainFormationWidget::render(U8G2 &display) {
  if (!isVisible || numCoaches == 0 || !formations)
    return;

  int noseConeWidth = 5; // 5-pixel base triangle overlapping frame by 1px
  int trainH = 11;       // 9 internal height + 2 borders
  int trainY = y + (height - trainH) / 2;
  int carriageWidth = 15; // 14 internal width + 1 divider boundary
  bool isMarquee = false;

  int totalTrainWidth = noseConeWidth + (numCoaches * carriageWidth);
  if (totalTrainWidth > width) {
    isMarquee = true;
  }

  int drawOffsetX = x + 1; // 1 pixel padding to prevent tip clipping

  if (!isMarquee && totalTrainWidth < width) {
    drawOffsetX = x + 1; // Left aligned with padding
  } else if (isMarquee) {
    display.setClipWindow(x, y, x + width, y + height);
    drawOffsetX = x + 1 + scrollOffset;
  }

  int currentX = drawOffsetX;

  // Draw the continuous bounding frame for all carriages FIRST
  int bodyWidth = numCoaches * carriageWidth;
  display.drawRFrame(currentX + noseConeWidth, trainY, bodyWidth, trainH, 2);

  // Draw Train Mask (Nose Cone - Right Angled Triangle) NEXT to overwrite left
  // frame edge
  display.setDrawColor(1);
  display.drawTriangle(currentX + 1, trainY + 10, currentX + 1 + noseConeWidth,
                       trainY - 3, currentX + 1 + noseConeWidth, trainY + 10);

  // Draw 11th line bumper (4 pixels wide, right-aligned)
  display.drawLine(currentX + 3, trainY + 10, currentX + 5, trainY + 10);

  // Draw vertical dividers between carriages
  for (int i = 1; i < numCoaches; ++i) {
    int divX = currentX + noseConeWidth + (i * carriageWidth);
    display.drawLine(divX, trainY, divX, trainY + trainH - 1);
  }

  currentX += noseConeWidth;

  for (int i = 0; i < numCoaches; ++i) {

    int innerW = 14;
    int innerX = currentX + 1; // Left boundary 1 px

    switch (currentMode) {
    case Mode::MODE_NUMBER: {
      char nm[4];
      snprintf(nm, sizeof(nm), "%d", formations[i].coachNumber);
      drawText(display, nm, innerX, trainY, innerW, trainH, TextAlign::CENTER,
               false, u8g2_font_5x7_tr, 2);
      break;
    }
    case Mode::MODE_FACILITIES: {
      int fCount = 0;
      const char *fArray[3];
      if (formations[i].coachClass == 'F' || formations[i].coachClass == 'f')
        fArray[fCount++] = "\xee\x80\x81";
      if (formations[i].hasWheelchair)
        fArray[fCount++] = "\xee\x80\x82";
      if (formations[i].hasBikes)
        fArray[fCount++] = "\xee\x80\x83";

      if (fCount > 0) {
        const char *displayIcon = fArray[(currentTickMs / 1500) % fCount];
        drawText(display, displayIcon, innerX, trainY, innerW, trainH,
                 TextAlign::CENTER, false, FormationIcons7, 8);
      }
      break;
    }
    case Mode::MODE_LOADING: {
      int pct = atoi(formations[i].loading);
      int fillHeight = (9 * pct) / 100; // max inner height = 9
      if (fillHeight > 9)
        fillHeight = 9;
      if (fillHeight > 0) {
        display.drawBox(innerX, (trainY + trainH - 1) - fillHeight, innerW,
                        fillHeight);
      }
      break;
    }
    }
    currentX += carriageWidth;
  }

  if (isMarquee) {
    display.setClipWindow(0, 0, 256, 64);
  }
}
