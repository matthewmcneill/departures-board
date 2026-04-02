/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/boards/systemBoard/firmwareUpdateBoard.cpp
 * Description: Implementation of the FirmwareUpdateBoard.
 *
 * Exported Functions/Classes:
 * - FirmwareUpdateBoard: [Class implementation]
 *   - init(): Injects dependencies into sub-boards.
 *   - setUpdateState() / setCountdownSeconds(): State management.
 *   - render() / tick(): Dispatch to msgBoard or loadBoard based on phase.
 */

#include "firmwareUpdateBoard.hpp"
#include <U8g2lib.h>
#include <appContext.hpp>

/**
 * @brief Construct a new Firmware Update Board.
 * Initializes state to WARNING phase.
 */
FirmwareUpdateBoard::FirmwareUpdateBoard() 
    : currentState(FwUpdateState::WARNING), countdownSeconds(0), downloadPercent(0) {
    releaseVersion[0] = '\0';
    errorMessage[0] = '\0';
}

/**
 * @brief Initialize the board and its dependencies.
 * @param contextPtr Pointer to the shared application context.
 */
void FirmwareUpdateBoard::init(appContext* contextPtr) {
    context = contextPtr;
    msgBoard.init(contextPtr);
    loadBoard.init(contextPtr);
}

/**
 * @brief Update the internal engine state.
 * @param state The new FwUpdateState.
 */
void FirmwareUpdateBoard::setUpdateState(FwUpdateState state) {
    currentState = state;
}

/**
 * @brief Set the visual countdown timer.
 * @param seconds Seconds remaining until transition.
 */
void FirmwareUpdateBoard::setCountdownSeconds(int seconds) {
    countdownSeconds = seconds;
}

/**
 * @brief Set the progress bar value.
 * @param percent Value between 0 and 100.
 */
void FirmwareUpdateBoard::setDownloadPercent(int percent) {
    downloadPercent = percent;
}

/**
 * @brief Set the version string for the update.
 * @param version Null-terminated version string.
 */
void FirmwareUpdateBoard::setReleaseVersion(const char* version) {
    if (version != nullptr) {
        strlcpy(releaseVersion, version, sizeof(releaseVersion));
    } else {
        releaseVersion[0] = '\0';
    }
}

/**
 * @brief Set the error message for the failure state.
 * @param error Null-terminated error string.
 */
void FirmwareUpdateBoard::setErrorMessage(const char* error) {
    if (error != nullptr) {
        strlcpy(errorMessage, error, sizeof(errorMessage));
    } else {
        errorMessage[0] = '\0';
    }
}

/**
 * @brief Propagate activation to managed sub-boards.
 */
void FirmwareUpdateBoard::onActivate() {
    msgBoard.onActivate();
    loadBoard.onActivate();
}

/**
 * @brief Propagate deactivation to managed sub-boards.
 */
void FirmwareUpdateBoard::onDeactivate() {
    msgBoard.onDeactivate();
    loadBoard.onDeactivate();
}

/**
 * @brief Periodic logic tick.
 * Dispatches to either MessageBoard or LoadingBoard based on state.
 * @param ms Milliseconds since last tick.
 */
void FirmwareUpdateBoard::tick(uint32_t ms) {
    if (currentState == FwUpdateState::DOWNLOADING) {
        loadBoard.tick(ms);
    } else {
        msgBoard.tick(ms);
    }
}

/**
 * @brief Handle per-frame animation logic.
 * @param display U8g2 reference.
 * @param currentMillis Wall clock time.
 */
void FirmwareUpdateBoard::renderAnimationUpdate(U8G2& display, uint32_t currentMillis) {
    if (currentState == FwUpdateState::DOWNLOADING) {
        loadBoard.renderAnimationUpdate(display, currentMillis);
    } else {
        msgBoard.renderAnimationUpdate(display, currentMillis);
    }
}

/**
 * @brief Primary render call.
 * Orchestrates multiple layouts (Warning, Download, Success, Failure) 
 * using sub-board components (MessageBoard and LoadingBoard).
 * @param display U8g2 reference.
 */
void FirmwareUpdateBoard::render(U8G2& display) {
    // --- Step 1: Prepare static data ---
    char countdownStr[60];
    char versionStr[64];

    switch (currentState) {
        case FwUpdateState::WARNING:
            snprintf(countdownStr, sizeof(countdownStr), "will be installed in %d seconds. This provides:", countdownSeconds);
            snprintf(versionStr, sizeof(versionStr), "\"%s\"", releaseVersion);
            msgBoard.setContent("Firmware Update Available", "A new version of the Departures Board firmware", countdownStr, versionStr, "* DO NOT REMOVE THE POWER DURING THE UPDATE *");
            msgBoard.render(display);
            break;

        case FwUpdateState::DOWNLOADING:
            loadBoard.setHeading("Departures Board");
            if (context) {
                loadBoard.setBuildTime(context->getBuildTime().c_str());
            }
            loadBoard.setProgress("Downloading Firmware", downloadPercent);
            loadBoard.render(display);
            break;

        case FwUpdateState::FAILED:
            msgBoard.setContent("Firmware Update", errorMessage, "Update process encountered an error.", "");
            msgBoard.render(display);
            break;

        case FwUpdateState::NO_UPDATES:
            msgBoard.setContent("Firmware Update", "", "No firmware updates were available.", "");
            msgBoard.render(display);
            break;

        case FwUpdateState::SUCCESS:
            snprintf(countdownStr, sizeof(countdownStr), "The board will restart in %d seconds...", countdownSeconds);
            msgBoard.setContent("Firmware Update", "The firmware update has completed successfully.", "For more information visit the URL below:", "github.com/gadec-uk/departures-board/releases", countdownStr);
            msgBoard.render(display);
            break;
    }
}
