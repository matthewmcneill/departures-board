/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/boards/systemBoard/firmwareUpdateBoard.hpp
 * Description: State-driven display board managing the OTA firmware update UI lifecycle.
 *
 * Exported Functions/Classes:
 * - FwUpdateState: Enumeration of possible update UI states.
 * - FirmwareUpdateBoard: Class extending iDisplayBoard for firmware updates.
 */

#ifndef FIRMWARE_UPDATE_BOARD_HPP
#define FIRMWARE_UPDATE_BOARD_HPP

#include "../interfaces/iDisplayBoard.hpp"
#include "../../widgets/drawingPrimitives.hpp"
#include <string.h>

/**
 * @brief Represents the current UI lifecycle state of a Firmware Update check or installation.
 */
enum class FwUpdateState {
    WARNING,
    DOWNLOADING,
    SUCCESS,
    FAILED,
    NO_UPDATES
};

#include "messageBoard.hpp"
#include "loadingBoard.hpp"

/**
 * @brief Board that orchestrates the UI for OTA Firmware Updates by formatting the underlying state properties.
 */
class FirmwareUpdateBoard : public iDisplayBoard {
private:
    MessageBoard msgBoard;
    LoadingBoard loadBoard;

    FwUpdateState currentState;
    int countdownSeconds;
    int downloadPercent;
    char releaseVersion[64];
    char errorMessage[64];

protected:
    FirmwareUpdateBoard();
    friend class DisplayManager;

public:

    /**
     * @brief Set the current firmware update state
     * @param state The new state (e.g. DOWNLOADING, WARNING)
     */
    void setUpdateState(FwUpdateState state);

    /**
     * @brief Set the countdown timer value
     * @param seconds Seconds remaining
     */
    void setCountdownSeconds(int seconds);

    /**
     * @brief Set the download progress
     * @param percent Progress between 0 and 100
     */
    void setDownloadPercent(int percent);

    /**
     * @brief Set the firmware release version string
     * @param version The new version tag
     */
    void setReleaseVersion(const char* version);

    /**
     * @brief Set the error message if an update fails
     * @param error Descriptive error message
     */
    void setErrorMessage(const char* error);

    // iDisplayBoard Implementation
    void onActivate() override;
    void onDeactivate() override;
    void tick(uint32_t ms) override;
    void render(U8G2& display) override;
    int updateData() override { return 0; }
    const char* getLastErrorMsg() override { return ""; }
    void renderAnimationUpdate(U8G2& display, uint32_t currentMillis) override;
};

#endif // FIRMWARE_UPDATE_BOARD_HPP
