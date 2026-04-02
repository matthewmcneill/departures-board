/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/boards/systemBoard/loadingBoard.cpp
 * Description: Implementation of the system startup and progress screen.
 *
 * Exported Functions/Classes:
 * - LoadingBoard: [Class implementation]
 *   - setProgress() / setHeading(): Visual progress and title updates.
 *   - setNotice() / setBuildTime(): Footer and status updates.
 *   - render(): Draw full screen layout including branding header.
 */

#include "loadingBoard.hpp"
#include <fonts/fonts.hpp>
#include <string.h>
#include "../../widgets/drawingPrimitives.hpp"
#include <logger.hpp>

/**
 * @brief Initialize the loading screen and its progress bar widget.
 */
LoadingBoard::LoadingBoard() 
    : pBar(0, 15, 256, 24) {
    noticeMessage[0] = '\0';
    heading[0] = '\0';
    buildTime[0] = '\0';
}

/**
 * @brief Update the visual progress and status message.
 * @param message String to display above/inside the bar.
 * @param percent 0-100 completion value.
 * @param durationMs Animation duration for the bar fill.
 */
void LoadingBoard::setProgress(const char* message, int percent, uint32_t durationMs) {
    if (message != nullptr) {
        strlcpy(noticeMessage, message, sizeof(noticeMessage));
        pBar.setMessage(message);
    }
    pBar.setPercent(percent, durationMs);
}

/**
 * @brief Set the large heading text.
 * @param newHeading The string text.
 */
void LoadingBoard::setHeading(const char* newHeading) {
    if (newHeading != nullptr) {
        strlcpy(heading, newHeading, sizeof(heading));
    }
}

/**
 * @brief Update the compilation timestamp shown in the footer.
 * @param newBuildTime The timestamp string.
 */
void LoadingBoard::setBuildTime(const char* newBuildTime) {
    if (newBuildTime != nullptr) {
        strlcpy(buildTime, newBuildTime, sizeof(buildTime));
    }
}

/**
 * @brief Set a secondary status notification string.
 */
void LoadingBoard::setNotice(const char* message) {
    if (message != nullptr) {
        strlcpy(noticeMessage, message, sizeof(noticeMessage));
    } else {
        noticeMessage[0] = '\0';
    }
}

/**
 * @brief Lifecycle hook for activation.
 */
void LoadingBoard::onActivate() {
    // Optional activation logic
}

/**
 * @brief Lifecycle hook for deactivation.
 */
void LoadingBoard::onDeactivate() {
    // Optional deactivation logic
}

/**
 * @brief Periodic logic tick for the progress bar.
 * @param ms Current system time in milliseconds.
 */
void LoadingBoard::tick(uint32_t ms) {
    pBar.tick(ms);
}

/**
 * @brief Partial render for high-speed progress bar animation.
 * @param display Reference to U8g2.
 * @param currentMillis Current system time in milliseconds.
 */
void LoadingBoard::renderAnimationUpdate(U8G2& display, uint32_t currentMillis) {
    pBar.renderAnimationUpdate(display, currentMillis); // hardware-accelerated bar fill
}

/**
 * @brief Draw the full loading screen including branding and progress.
 * @param display Reference to U8g2.
 */
void LoadingBoard::render(U8G2& display) {
    // --- Step 1: Branding Header ---
    if (heading[0] != '\0') {
        drawText(display, heading, 0, 0, 256, 16, TextAlign::CENTER, false, NatRailTall12);
    }

    // --- Step 2: Progress Integration ---
    // The pBar widget handles clearing its own area and drawing the noticeMessage.
    pBar.render(display);
    
    // --- Step 3: Build Metadata ---
    display.setFont(NatRailSmall9);
    
    // Version label (Left aligned)
    display.drawStr(2, 53, "Version: 3.0");

    // Compiled Timestamp (Right aligned)
    if (buildTime[0] != '\0') {
        char buildStr[32];
        snprintf(buildStr, sizeof(buildStr), "Build: %s", buildTime);
        int textW = display.getStrWidth(buildStr);
        display.drawStr(256 - textW - 2, 53, buildStr);
    }
}
