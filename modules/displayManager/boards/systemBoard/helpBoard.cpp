/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/boards/systemBoard/helpBoard.cpp
 * Description: Implementation of the Help Board.
 */

#include "helpBoard.hpp"
#include <fonts/fonts.hpp>
#include "../../widgets/drawingPrimitives.hpp"
#include <string.h>

HelpBoard::HelpBoard() : lineCount(0) {
    title[0] = '\0';
    for (int i = 0; i < 5; i++) {
        lines[i] = nullptr;
    }
}

void HelpBoard::setHelpContent(const char* h, const char* textArr[], int count) {
    if (h) strlcpy(title, h, sizeof(title));
    lineCount = (count > 5) ? 5 : count; // clamp
    for (int i = 0; i < lineCount; i++) {
        lines[i] = textArr[i];
    }
}

void HelpBoard::onActivate() {
}

void HelpBoard::onDeactivate() {
}

void HelpBoard::tick(uint32_t ms) {
}

void HelpBoard::render(U8G2& display) {
    display.setFont(NatRailSmall9);
    
    if (title[0] != '\0') {
        drawText(display, title, 0, 3, 256, 10, TextAlign::CENTER);
    }
    
    int startY = 14;
    int spacing = 10;
    
    for (int i = 0; i < lineCount; i++) {
        if (lines[i] != nullptr) {
            drawText(display, lines[i], 0, startY + (i * spacing), 256, 10, TextAlign::LEFT, true);
        }
    }
}
