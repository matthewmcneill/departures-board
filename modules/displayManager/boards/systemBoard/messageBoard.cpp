/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/boards/systemBoard/messageBoard.cpp
 * Description: Implementation of the configurable alert message board.
 */

#include "messageBoard.hpp"
#include <fonts/fonts.hpp>
#include "../../widgets/drawingPrimitives.hpp"

MessageBoard::MessageBoard() : showWarningIcon(false) {
    hdr[0] = '\0';
    title[0] = '\0';
    msg1[0] = '\0';
    msg2[0] = '\0';
    msg3[0] = '\0';
    msg4[0] = '\0';
}

void MessageBoard::setContent(const char* h, const char* t, 
                              const char* m1, const char* m2, 
                              const char* m3, const char* m4) {
    if (h) strlcpy(hdr, h, sizeof(hdr)); else hdr[0] = '\0';
    if (t) strlcpy(title, t, sizeof(title)); else title[0] = '\0';
    if (m1) strlcpy(msg1, m1, sizeof(msg1)); else msg1[0] = '\0';
    if (m2) strlcpy(msg2, m2, sizeof(msg2)); else msg2[0] = '\0';
    if (m3) strlcpy(msg3, m3, sizeof(msg3)); else msg3[0] = '\0';
    if (m4) strlcpy(msg4, m4, sizeof(msg4)); else msg4[0] = '\0';
}

void MessageBoard::setWarningIcon(bool show) {
    showWarningIcon = show;
}

void MessageBoard::onActivate() {
    // Optional activation hook
}

void MessageBoard::onDeactivate() {
    // Optional deactivation hook
}

void MessageBoard::tick(uint32_t ms) {
    // Message screens are usually static, no logic per tick
}

void MessageBoard::render(U8G2& display) {
    
    if (hdr[0] != '\0') {
        drawText(display, hdr, 0, 3, 256, 10, TextAlign::CENTER, false, NatRailSmall9);
    }
    
    if (title[0] != '\0') {
        drawText(display, title, 0, 20, 256, 14, TextAlign::CENTER, false, NatRailTall12); 
    }

    display.setFont(NatRailSmall9);
    
    // Auto-flow layout for body lines
    int startY = 38;
    int spacing = 10;
    
    if (msg1[0] != '\0') drawText(display, msg1, 0, startY, 256, 10, TextAlign::CENTER);
    if (msg2[0] != '\0') drawText(display, msg2, 0, startY + spacing, 256, 10, TextAlign::CENTER);
    if (msg3[0] != '\0') drawText(display, msg3, 0, startY + (spacing * 2), 256, 10, TextAlign::CENTER);
    if (msg4[0] != '\0') drawText(display, msg4, 0, startY + (spacing * 3), 256, 10, TextAlign::CENTER);
    
    if (showWarningIcon) {
        display.setFont(NatRailTall12);
        display.drawStr(0, 50, "}"); // Ensure '}' maps to warning glyph in your icon font
        display.setFont(NatRailSmall9);
    }
}
