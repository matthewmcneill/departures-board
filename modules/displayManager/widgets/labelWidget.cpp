/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/widgets/labelWidget.cpp
 * Description: Implementation of the labelWidget class. Handles string boundary 
 *              calculations, ellipsis truncation, and U8G2 layout alignments.
 */

#include "labelWidget.hpp"
#include "drawingPrimitives.hpp"
#include <string.h>

labelWidget::labelWidget(int _x, int _y, int _w, int _h) 
    : iGfxWidget(_x, _y, _w, _h), font(nullptr), alignment(0), isTruncated(false) {
    memset(textBuffer, 0, sizeof(textBuffer));
}

void labelWidget::setText(const char* text) {
    if (text) {
        strncpy(textBuffer, text, sizeof(textBuffer) - 1);
        textBuffer[sizeof(textBuffer) - 1] = '\0';
    } else {
        textBuffer[0] = '\0';
    }
}

void labelWidget::setFont(const uint8_t* newFont) {
    font = newFont;
}

void labelWidget::setAlignment(int align) {
    alignment = align;
}

void labelWidget::setTruncated(bool truncated) {
    isTruncated = truncated;
}

void labelWidget::tick(uint32_t currentMillis) {
    // labelWidget currently utilizes static geometry with no ongoing animation mechanics
    (void)currentMillis;
}

void labelWidget::render(U8G2& display) {
    if (!isVisible || !font || textBuffer[0] == '\0') {
        return;
    }

    display.setFont(font);

    // Make an isolated copy or work buffer in case we need to mutate it for truncation
    char renderBuffer[128];
    strncpy(renderBuffer, textBuffer, sizeof(renderBuffer) - 1);
    renderBuffer[sizeof(renderBuffer) - 1] = '\0';

    int txtWidth = getStringWidth(display, renderBuffer);
    
    // Evaluate truncation if defined and if widget width applies (width > 0)
    if (isTruncated && width > 0 && txtWidth > width) {
        size_t len = strlen(renderBuffer);
        int ellipsisWidth = getStringWidth(display, "...");
        
        while (len > 0 && (getStringWidth(display, renderBuffer) + ellipsisWidth > width)) {
            len--;
            renderBuffer[len] = '\0';
        }
        
        strncat(renderBuffer, "...", sizeof(renderBuffer) - strlen(renderBuffer) - 1);
        txtWidth = getStringWidth(display, renderBuffer); // Refresh calculated width
    }

    // Evaluate Alignment logic
    int renderX = x;
    if (alignment == 1) { // Center
        if (width > 0) {
            renderX = x + (width - txtWidth) / 2;
        } else {
            // Default center around X without arbitrary bound width
            renderX = x - (txtWidth / 2);
        }
    } else if (alignment == 2) { // Right alignment
        if (width > 0) {
            renderX = x + width - txtWidth;
        } else {
            renderX = x - txtWidth;
        }
    }

    display.drawStr(renderX, y, renderBuffer);
}
