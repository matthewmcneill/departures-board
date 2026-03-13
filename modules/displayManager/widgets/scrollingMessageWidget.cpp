/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/gfxUtilities/src/scrollingMessageWidget.cpp
 * Description: Implementation of horizontal marquee scrolling logic.
 */

#include "scrollingMessageWidget.hpp"
#include "drawingPrimitives.hpp"
#include <displayManager.hpp>

extern DisplayManager displayManager;

scrollingMessageWidget::scrollingMessageWidget(int _x, int _y, int _w, int _h, const uint8_t* _font)
    : iGfxWidget(_x, _y, _w, _h), scrollX(0), messageWidth(0), lastScrollMs(0), needsScroll(false), needsLayout(true), font(_font) {
    if (font == nullptr) font = NatRailSmall9;
    message[0] = '\0';
}

void scrollingMessageWidget::setMessage(const char* newMessage) {
    if (newMessage == nullptr) return;
    if (strcmp(message, newMessage) == 0) return;

    strncpy(message, newMessage, sizeof(message) - 1);
    message[sizeof(message) - 1] = '\0';
    
    needsLayout = true;
}

void scrollingMessageWidget::resetScroll() {
    scrollX = 0;
    lastScrollMs = 0; // Pause at start
}

void scrollingMessageWidget::tick(uint32_t currentMillis) {
    if (!isVisible || !needsScroll) return;

    if (lastScrollMs == 0) {
        lastScrollMs = currentMillis + 3000; // Pause at start
        return;
    }

    if (currentMillis > lastScrollMs) {
        scrollX++;
        int maxW = (width > 0) ? width : SCREEN_WIDTH;
        
        if (scrollX > messageWidth + 30) {
            scrollX = -maxW; // Scroll back in from right
        }
        
        lastScrollMs = currentMillis + 40; // ~25fps
    }
}

void scrollingMessageWidget::render(U8G2& display) {
    if (!isVisible || message[0] == '\0') return;

    if (needsLayout) {
        display.setFont(font);
        messageWidth = getStringWidth(display, message);
        int maxW = (width > 0) ? width : SCREEN_WIDTH;
        needsScroll = (messageWidth > maxW);
        resetScroll();
        needsLayout = false;
    }

    int renderW = (width > 0) ? width : SCREEN_WIDTH;
    int renderH = (height > 0) ? height : 12;

    blankArea(display, x, y, renderW, renderH);

    display.setFont(font);
    
    if (needsScroll) {
        display.setClipWindow(x, y, x + renderW, y + renderH);
        display.drawStr(x - scrollX, y + renderH - 2, message);
        display.setMaxClipWindow();
    } else {
        // Center if it fits but we want consistency or just draw it?
        // Boards usually center these messages.
        int w = display.getStrWidth(message);
        int start = x + (renderW - w) / 2;
        display.drawStr(start, y + renderH - 2, message);
    }
}

void scrollingMessageWidget::renderAnimationUpdate(U8G2& display, uint32_t currentMillis) {
    if (!isVisible || message[0] == '\0') return;

    if (needsLayout) {
        display.setFont(font);
        messageWidth = getStringWidth(display, message);
        int maxW = (width > 0) ? width : SCREEN_WIDTH;
        needsScroll = (messageWidth > maxW);
        resetScroll();
        needsLayout = false;
    }

    if (!needsScroll) return;

    int oldScrollX = scrollX;
    tick(currentMillis); // Deduplicate state math

    if (scrollX != oldScrollX) {
        int renderW = (width > 0) ? width : SCREEN_WIDTH;
        int renderH = (height > 0) ? height : 12;

        display.setClipWindow(x, y, x + renderW, y + renderH);
        blankArea(display, x, y, renderW, renderH);

        display.setFont(font);
        display.drawStr(x - scrollX, y + renderH - 2, message);
        display.setMaxClipWindow();

        display.updateDisplayArea(x / 8, y / 8, (renderW + 7) / 8, (renderH + 7) / 8);
    }
}
