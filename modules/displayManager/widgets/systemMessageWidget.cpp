#include <departuresBoard.hpp>
#include <fonts/fonts.hpp>
/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/widgets/systemMessageWidget.cpp
 * Description: Implementation of centered multi-line alert layouts.
 */

#include "systemMessageWidget.hpp"
#include "drawingPrimitives.hpp"

/**
 * @brief Initialize the system message widget with default state.
 */
systemMessageWidget::systemMessageWidget(int _x, int _y, int _w, int _h) 
    : iGfxWidget(_x, _y, _w, _h), numLines(0) {
    title[0] = '\0';
    lines[0][0] = '\0';
    lines[1][0] = '\0';
    lines[2][0] = '\0';
}

/**
 * @brief Bulk update the text lines.
 * @param newTitle Main heading.
 * @param l1 Line 1 content.
 * @param l2 Line 2 content.
 * @param l3 Line 3 content.
 */
void systemMessageWidget::setMessage(const char* newTitle, const char* l1, const char* l2, const char* l3) {
    if (newTitle) strncpy(title, newTitle, sizeof(title)-1);
    else title[0] = '\0';
    
    numLines = 0;
    if (l1) { strncpy(lines[0], l1, sizeof(lines[0])-1); numLines = 1; }
    if (l2) { strncpy(lines[1], l2, sizeof(lines[1])-1); numLines = 2; }
    if (l3) { strncpy(lines[2], l3, sizeof(lines[2])-1); numLines = 3; }
}

/**
 * @brief Draw the centered notification text.
 * @param display U8g2 reference.
 */
void systemMessageWidget::render(U8G2& display) {
    if (!isVisible) return;

    int renderW = (width > 0) ? width : SCREEN_WIDTH;
    int renderH = (height > 0) ? height : 14;

    blankArea(display, x, y, renderW, renderH);

    // Title
    drawText(display, title, x, y + 12, renderW, -1, TextAlign::CENTER, false, Underground10);

    // Lines
    int yOffset = y + 28;
    for (int i = 0; i < numLines; i++) {
        drawText(display, lines[i], x, yOffset, renderW, -1, TextAlign::CENTER, false, NatRailSmall9);
        yOffset += 12;
    }
}
