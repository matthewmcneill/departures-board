/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/boards/systemBoard/splashBoard.cpp
 * Description: Implementation of the startup splash screen.
 */

#include "splashBoard.hpp"
#include <U8g2lib.h>

SplashBoard::SplashBoard() {
    noticeMessage[0] = '\0';
    // Instantiate the graphic at runtime
    splashLogo = new imageWidget(81, 0, gadeclogo_width, gadeclogo_height, gadeclogo_bits);
    
    // Instantiate the progress bar at the bottom center
    bootProgressBar = new progressBarWidget(58, 52, 140, 8);
    bootProgressBar->setShowPercentText(false);
}

SplashBoard::~SplashBoard() {
    delete splashLogo;
    delete bootProgressBar;
}

void SplashBoard::setNotice(const char* message) {
    if (message != nullptr) {
        strlcpy(noticeMessage, message, sizeof(noticeMessage));
    } else {
        noticeMessage[0] = '\0';
    }
}

void SplashBoard::setProgress(int percentage) {
    if (bootProgressBar) {
        bootProgressBar->setPercent(percentage);
    }
}

void SplashBoard::onActivate() {
    // Optional activation logic
}

void SplashBoard::onDeactivate() {
    // Optional deactivation logic
}

void SplashBoard::tick(uint32_t ms) {
    // Optional periodic tasks
}

void SplashBoard::render(U8G2& display) {
    // Draw the image widget
    if (splashLogo) {
        splashLogo->render(display);
    }
    
    // Draw the centred notice text below the logo
    display.setFont(NatRailSmall9);
    if (noticeMessage[0] != '\0') {
        display.setFont(NatRailSmall9);
        centreText(display, noticeMessage, 48);
    }
    
    // Draw the progress bar widget
    if (bootProgressBar) {
        bootProgressBar->render(display);
    }
}
