/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons
 * Attribution-NonCommercial-ShareAlike 4.0 International. To view a copy of
 * this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/boards/systemBoard/firmwareUpdateBoard.hpp
 * Description: State-driven display board managing the OTA firmware update UI lifecycle.
 *
 * Exported Functions/Classes:
 * - FirmwareUpdateBoard: [Class] Orchestrates the UI for OTA Firmware Updates.
 *   - setUpdateState(): Updates the current UI phase (e.g. DOWNLOADING).
 *   - setCountdownSeconds(): Sets the restart/transition timer.
 *   - setDownloadPercent(): Sets the progress bar value (0-100).
 *   - setReleaseVersion(): Injects the version string being installed.
 *   - setErrorMessage(): Injects failure text for the error state.
 *   - init(): Service injection point.
 *   - onActivate() / onDeactivate(): Display lifecycle hooks.
 *   - tick() / render(): Logic and drawing entry points.
 */

#ifndef FIRMWARE_UPDATE_BOARD_HPP
#define FIRMWARE_UPDATE_BOARD_HPP

class appContext;

#include "../interfaces/iDisplayBoard.hpp"
#include <otaUpdateManager.hpp>

#include "loadingBoard.hpp"
#include "messageBoard.hpp"

/**
 * @brief Board that orchestrates the UI for OTA Firmware Updates by formatting
 * the underlying state properties.
 */
class FirmwareUpdateBoard : public iDisplayBoard {
private:
  appContext *context;
  MessageBoard msgBoard;
  LoadingBoard loadBoard;

  FwUpdateState currentState;
  int countdownSeconds;
  int downloadPercent;
  char releaseVersion[64];
  char errorMessage[64];
  WeatherStatus weatherStatus;

protected:
  FirmwareUpdateBoard();
  friend class BoardFactory;

public:
  const char *getBoardName() const override { return "SYS: Firmware Update"; }

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
  void setReleaseVersion(const char *version);

  /**
   * @brief Set the error message if an update fails
   * @param error Descriptive error message
   */
  void setErrorMessage(const char *error);

  void init(appContext *contextPtr);

  // iDisplayBoard Implementation
  void onActivate() override;
  void onDeactivate() override;
  void configure(const struct BoardConfig &config) override { (void)config; }
  void tick(uint32_t ms) override;
  void render(U8G2 &display) override;
  UpdateStatus updateData() override { return UpdateStatus::SUCCESS; }
  const char *getLastErrorMsg() override { return ""; }
  void renderAnimationUpdate(U8G2 &display, uint32_t currentMillis) override;
  WeatherStatus &getWeatherStatus() override { return weatherStatus; }
};

#endif // FIRMWARE_UPDATE_BOARD_HPP
