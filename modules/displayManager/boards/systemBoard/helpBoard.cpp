/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/boards/systemBoard/helpBoard.cpp
 * Description: Implementation of the Help Board.
 *
 * Exported Functions/Classes:
 * - HelpBoard: [Class implementation]
 *   - setHelpContent(): Clamps and assigns the instructional text.
 *   - render(): Draws centered header and left-aligned body text.
 */

#include "helpBoard.hpp"
#include <fonts/fonts.hpp>
#include "../../widgets/drawingPrimitives.hpp"
#include <string.h>

/**
 * @brief Construct a new Help Board.
 * Zero-initializes line pointers and title.
 */
HelpBoard::HelpBoard() : lineCount(0) {
    title[0] = '\0';
    for (int i = 0; i < 5; i++) {
        lines[i] = nullptr;
    }
}

/**
 * @brief Update the displayed text lines.
 * @param h Header title.
 * @param textArr Array of string pointers for the body.
 * @param count Number of elements in textArr.
 */
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

/**
 * @brief Periodic logic tick (no-op for help board).
 */
void HelpBoard::tick(uint32_t ms) {
}

/**
 * @brief Primary render pass.
 * Draws the centered title and left-aligned body lines.
 * @param display Global U8g2 reference.
 */
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
