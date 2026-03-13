/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/boards/systemBoard/firmwareUpdateBoard.cpp
 * Description: Implementation of the FirmwareUpdateBoard.
 */

#include "firmwareUpdateBoard.hpp"
#include "systemBoard.hpp"
#include <U8g2lib.h>

FirmwareUpdateBoard::FirmwareUpdateBoard() 
    : currentState(FwUpdateState::WARNING), countdownSeconds(0), downloadPercent(0) {
    releaseVersion[0] = '\0';
    errorMessage[0] = '\0';
}

void FirmwareUpdateBoard::setUpdateState(FwUpdateState state) {
    currentState = state;
}

void FirmwareUpdateBoard::setCountdownSeconds(int seconds) {
    countdownSeconds = seconds;
}

void FirmwareUpdateBoard::setDownloadPercent(int percent) {
    downloadPercent = percent;
}

void FirmwareUpdateBoard::setReleaseVersion(const char* version) {
    if (version != nullptr) {
        strlcpy(releaseVersion, version, sizeof(releaseVersion));
    } else {
        releaseVersion[0] = '\0';
    }
}

void FirmwareUpdateBoard::setErrorMessage(const char* error) {
    if (error != nullptr) {
        strlcpy(errorMessage, error, sizeof(errorMessage));
    } else {
        errorMessage[0] = '\0';
    }
}

void FirmwareUpdateBoard::onActivate() {
    msgBoard.onActivate();
    loadBoard.onActivate();
}

void FirmwareUpdateBoard::onDeactivate() {
    msgBoard.onDeactivate();
    loadBoard.onDeactivate();
}

void FirmwareUpdateBoard::tick(uint32_t ms) {
    if (currentState == FwUpdateState::DOWNLOADING) {
        loadBoard.tick(ms);
    } else {
        msgBoard.tick(ms);
    }
}

void FirmwareUpdateBoard::renderAnimationUpdate(U8G2& display, uint32_t currentMillis) {
    if (currentState == FwUpdateState::DOWNLOADING) {
        loadBoard.renderAnimationUpdate(display, currentMillis);
    } else {
        msgBoard.renderAnimationUpdate(display, currentMillis);
    }
}

void FirmwareUpdateBoard::render(U8G2& display) {
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
            loadBoard.setBuildTime(getBuildTime().c_str());
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
