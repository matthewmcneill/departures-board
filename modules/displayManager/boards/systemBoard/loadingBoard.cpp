/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/boards/systemBoard/loadingBoard.cpp
 * Description: Implementation of the loading screen.
 */

#include "loadingBoard.hpp"
#include <string.h>
#include "../../widgets/drawingPrimitives.hpp"
#include <logger.hpp>

LoadingBoard::LoadingBoard() 
    : pBar(0, 15, 256, 17) {
    noticeMessage[0] = '\0';
    heading[0] = '\0';
    buildTime[0] = '\0';
}

void LoadingBoard::setProgress(const char* message, int percent) {
    if (message != nullptr) {
        strlcpy(noticeMessage, message, sizeof(noticeMessage));
        pBar.setMessage(message);
    }
    pBar.setPercent(percent);
}

void LoadingBoard::setHeading(const char* newHeading) {
    if (newHeading != nullptr) {
        strlcpy(heading, newHeading, sizeof(heading));
    }
}

void LoadingBoard::setBuildTime(const char* newBuildTime) {
    if (newBuildTime != nullptr) {
        strlcpy(buildTime, newBuildTime, sizeof(buildTime));
    }
}

void LoadingBoard::setNotice(const char* message) {
    if (message != nullptr) {
        strlcpy(noticeMessage, message, sizeof(noticeMessage));
    } else {
        noticeMessage[0] = '\0';
    }
}

void LoadingBoard::onActivate() {
    // Optional activation logic
}

void LoadingBoard::onDeactivate() {
    // Optional deactivation logic
}

void LoadingBoard::tick(uint32_t ms) {
    pBar.tick(ms);
}

void LoadingBoard::renderAnimationUpdate(U8G2& display, uint32_t currentMillis) {
    pBar.renderAnimationUpdate(display, currentMillis); // hardware-accelerated bar fill
}

void LoadingBoard::render(U8G2& display) {
    // Draw heading at the top
    display.setFont(NatRailTall12);
    if (heading[0] != '\0') {
        centreText(display, heading, 0);
    }
    
    // Draw build time at bottom right
    display.setFont(NatRailSmall9);
    if (buildTime[0] != '\0') {
        display.drawStr(0, 53, buildTime);
    }
    
    // Draw progress bar logic in the middle block
    pBar.render(display);
    
    // Draw notice message at the very bottom if provided
    display.setFont(NatRailSmall9);
    if (noticeMessage[0] != '\0') {
        centreText(display, noticeMessage, 53);
    }
}
