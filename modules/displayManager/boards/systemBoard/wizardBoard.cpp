/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/boards/systemBoard/wizardBoard.cpp
 * Description: Implementation of the Wizard config board.
 *
 * Exported Functions/Classes:
 * - WizardBoard: [Class implementation]
 *   - setWizardIp(): Updates the captive portal IP address for display.
 *   - onActivate(): Resets the rotation timer.
 *   - tick(): Logic-only state rotation (8-second stages).
 *   - render(): Draws branding, SSID, and connection URL.
 */

#include <fonts/fonts.hpp>
#include "wizardBoard.hpp"
#include <appContext.hpp>
#include <logger.hpp>

WizardBoard::WizardBoard() : currentIp(0,0,0,0), lastStageSwitch(0), currentStage(0) {
}

/**
 * @brief Sets the IP address to be displayed by the wizard.
 *
 * @param ip The IP address to display.
 */
/**
 * @brief Set the IP address shown to the user.
 * @param ip IPAddress object from the WiFi manager.
 */
void WizardBoard::setWizardIp(IPAddress ip) {
    currentIp = ip;
}

/**
 * @brief Called when the board is activated.
 *
 * Resets the stage timer.
 */
void WizardBoard::onActivate() {
    lastStageSwitch = millis();
}

/**
 * @brief Called when the board is deactivated.
 *
 * No action required for this board.
 */
void WizardBoard::onDeactivate() {
}

/**
 * @brief Updates the board's state.
 *
 * Rotates through display stages every 8 seconds.
 *
 * @param ms The current system milliseconds.
 */
void WizardBoard::tick(uint32_t ms) {
    if (ms - lastStageSwitch > 8000) {
        currentStage = (currentStage + 1) % 3;
        lastStageSwitch = ms;
    }
}

/**
 * @brief Main rendering entry point for the wizard.
 * Renders setup branding, instructions, and connection details.
 * @param display Global U8G2 graphics instance.
 */
void WizardBoard::render(U8G2& display) {
    drawText(display, "WiFi Setup Mode", 0, 0, 256, 16, TextAlign::CENTER, false, NatRailTall12);
    
    display.setFont(NatRailSmall9);
    
    // Line 1: SSID instructions
    char ssidMsg[64] = "Join WiFi: ";
    if (context) {
        strlcat(ssidMsg, context->getWifiManager().getSSID(), sizeof(ssidMsg));
        if (strlen(context->getWifiManager().getSSID()) == 0) {
            strlcat(ssidMsg, "Departures-Board", sizeof(ssidMsg));
        }
    } else {
        strlcat(ssidMsg, "Departures-Board", sizeof(ssidMsg));
    }
    drawText(display, ssidMsg, 0, 24, 256, 12, TextAlign::CENTER);

    // Line 2: URL instructions
    char address[48];
    if (currentIp[0] != 0) {
        snprintf(address, sizeof(address), "Browse: http://%d.%d.%d.%d", currentIp[0], currentIp[1], currentIp[2], currentIp[3]);
    } else {
        snprintf(address, sizeof(address), "Connect to configure...");
    }
    drawText(display, address, 0, 40, 256, 12, TextAlign::CENTER);
    
    display.setFont(NatRailSmall9);
    drawText(display, "Follow instructions on your phone", 0, 54, 256, 10, TextAlign::LEFT, true);
}
