/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/boards/systemBoard/splashBoard.cpp
 * Description: Implementation of the startup splash screen.
 *
 * Exported Functions/Classes:
 * - SplashBoard: [Class implementation]
 *   - setNotice(): Updates the copyright/status text safely.
 *   - render(): Draws the branding logo and centered notice text.
 */

#include "splashBoard.hpp"
#include <fonts/fonts.hpp>
#include <U8g2lib.h>

/**
 * @brief Construct a new Splash Board.
 * Instantiates the xbm image widget for the branding logo.
 */
SplashBoard::SplashBoard() {
    noticeMessage[0] = '\0';
    // Instantiate the graphic at runtime
    splashLogo = std::make_unique<imageWidget>(81, 0, gadeclogo_width, gadeclogo_height, gadeclogo_bits);
}

SplashBoard::~SplashBoard() {
}

/**
 * @brief Update the status message text.
 * @param message Null-terminated string (e.g. copyright info).
 */
void SplashBoard::setNotice(const char* message) {
    if (message != nullptr) {
        strlcpy(noticeMessage, message, sizeof(noticeMessage));
    } else {
        noticeMessage[0] = '\0';
    }
}

void SplashBoard::onActivate() {
    // Optional activation logic
}

void SplashBoard::onDeactivate() {
    // Optional deactivation logic
}

void SplashBoard::tick(uint32_t ms) {
}

/**
 * @brief Primary render pass for the splash screen.
 * Draws the branding logo and copyright notice text.
 * @param display Global U8G2 graphics instance.
 */
void SplashBoard::render(U8G2& display) {
    // Draw the image widget
    if (splashLogo) {
        splashLogo->render(display);
    }
    
    // Draw the centred notice text below the logo
    display.setFont(NatRailSmall9);
    if (noticeMessage[0] != '\0') {
        drawText(display, noticeMessage, 0, 44, 256, 10, TextAlign::CENTER);
    }
}
