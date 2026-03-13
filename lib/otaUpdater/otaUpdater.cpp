/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/otaUpdater/otaUpdater.cpp
 * Description: Implementation of firmware lifecycle targeting GitHub releases.
 *
 * Exported Functions/Classes:
 * - isFirmwareUpdateAvailable: Logical version comparison helper.
 * - OtaUpdater::tick: Asynchronous maintenance loop.
 * - OtaUpdater::checkForFirmwareUpdate: Full binary download and flash sequence.
 * - ota: Global orchestration instance.
 */

#include "otaUpdater.hpp"
#include <HTTPClient.h>
#include <HTTPUpdateGitHub.h>
#include <githubClient.h>
#include <time.h>
#include <displayManager.hpp>
#include <Logger.hpp>
#include <boards/systemBoard/systemBoard.hpp>
#include <boards/systemBoard/firmwareUpdateBoard.hpp>
#include "../../include/webgui/index.h"

extern struct tm timeinfo;       // Global synchronized time struct
extern DisplayManager displayManager; // Global display orchestrator

OtaUpdater ota;                       // Global OTA maintenance instance

OtaUpdater::OtaUpdater() : prevUpdateCheckDay(-1), fwUpdateCheckTimer(0), updatesEnabled(false), dailyCheckEnabled(false) {
}

/**
 * @brief Main execution tick to check for updates periodically.
 *        Implements a randomized/jittered check once per day if enabled.
 */
void OtaUpdater::tick() {
    // --- Step 1: Check Throttling and Configuration ---
    if (dailyCheckEnabled && millis() > fwUpdateCheckTimer) {
        // Debounce subsequent checks for ~1 hour to avoid API spamming
        fwUpdateCheckTimer = millis() + 3300000 + random(600000); 
        
        // --- Step 2: Trigger Daily Logic ---
        if (timeinfo.tm_mday != prevUpdateCheckDay) {
            if (ghUpdate.getLatestRelease()) {
                checkForFirmwareUpdate();
                prevUpdateCheckDay = timeinfo.tm_mday;
            }
        }
    }
}

/**
 * @brief Callback function for displaying firmware update progress mapped to the Library callback
 */
void update_progress(int cur, int total) {
  int percent = (cur*100)/total;
  FirmwareUpdateBoard* fwBoard = (FirmwareUpdateBoard*)displayManager.getSystemBoard(SystemBoardId::SYS_FIRMWARE_UPDATE);
  if (fwBoard) {
    fwBoard->setUpdateState(FwUpdateState::DOWNLOADING);
    fwBoard->setDownloadPercent(percent);
    displayManager.render();
  }
}

/**
 * @brief Check GitHub if a newer firmware than our VERSION_MAJOR and VERSION_MINOR exists
 * @return True if a newer version is found.
 */
bool isFirmwareUpdateAvailable() {
  int releaseMajor = ghUpdate.releaseId.substring(1,ghUpdate.releaseId.indexOf(".")).toInt();
  int releaseMinor = ghUpdate.releaseId.substring(ghUpdate.releaseId.indexOf(".")+1,ghUpdate.releaseId.indexOf("-")).toInt();
  if (VERSION_MAJOR > releaseMajor) return false;
  if ((VERSION_MAJOR == releaseMajor) && (VERSION_MINOR >= releaseMinor)) return false;
  return true;
}

/**
 * @brief Attempts to install newer firmware if available and reboots.
 *        Executes a multi-stage sequence: Discovery -> User Warning -> Download -> Flash.
 * @return True if process completes successfully.
 */
bool OtaUpdater::checkForFirmwareUpdate() {
  bool result = true;

  // --- Step 1: Resolve Metadata ---
  if (!isFirmwareUpdateAvailable()) return result;

  // Find the firmware binary in the release assets
  String updatePath="";
  for (int i=0;i<ghUpdate.releaseAssets;i++){
    if (ghUpdate.releaseAssetName[i] == "firmware.bin") {
      updatePath = ghUpdate.releaseAssetURL[i];
      break;
    }
  }
  if (updatePath.length()==0) return result;

  // --- Step 2: Notify User (Countdown Warning) ---
  FirmwareUpdateBoard* fwBoard = (FirmwareUpdateBoard*)displayManager.getSystemBoard(SystemBoardId::SYS_FIRMWARE_UPDATE);
  if (!fwBoard) return result;

  displayManager.showBoard(fwBoard);

  unsigned long tmr=millis()+1000;
  for (int i=30;i>=0;i--) {
    fwBoard->setUpdateState(FwUpdateState::WARNING);
    fwBoard->setCountdownSeconds(i);
    fwBoard->setReleaseVersion(ghUpdate.releaseDescription.c_str());
    displayManager.showBoard(fwBoard);

    while (tmr>millis()) {
      yield(); // Allow background networking to breathe during countdown
    }
    tmr=millis()+1000;
  }
  
  // --- Step 3: Initiate Binary Download and Flash ---
  update_progress(0, 100);  // Initial progress flash
  WiFiClientSecure client;
  client.setInsecure();
  httpUpdate.onProgress(update_progress);
  httpUpdate.rebootOnUpdate(false); 

  HTTPUpdateResult ret = httpUpdate.handleUpdate(client, updatePath, ghUpdate.accessToken);
  
  // --- Step 4: Handle Result and Finalize ---
  switch (ret) {
    case HTTP_UPDATE_FAILED:
      result=false;
      for (int i=20;i>=0;i--) {
        fwBoard->setUpdateState(FwUpdateState::FAILED);
        fwBoard->setErrorMessage(httpUpdate.getLastErrorString().c_str());
        displayManager.showBoard(fwBoard);
        delay(1000);
      }
      break;

    case HTTP_UPDATE_NO_UPDATES:
      for (int i=10;i>=0;i--) {
        fwBoard->setUpdateState(FwUpdateState::NO_UPDATES);
        displayManager.showBoard(fwBoard);
        delay(1000);
      }
      break;

    case HTTP_UPDATE_OK:
      for (int i=20;i>=0;i--) {
        fwBoard->setUpdateState(FwUpdateState::SUCCESS);
        fwBoard->setCountdownSeconds(i);
        displayManager.showBoard(fwBoard);
        delay(1000);
      }
      displayManager.resetState();
      ESP.restart(); // Complete hardware lifecycle
      break;
  }
  
  displayManager.resetState();
  return result;
}

/**
 * @brief Checks for a version mismatch between the running firmware's expected
 *        Web UI and what is currently on disk. Performs cleanup if upgraded.
 */
void OtaUpdater::checkPostWebUpgrade() {
  String prevGUI = "";
  File f = LittleFS.open(F("/webver"), "r");
  if (f) {
    prevGUI = f.readString();
    f.close();
  }
  prevGUI.trim();
  
  String currentGUI = String(WEBAPPVER_MAJOR) + F(".") + String(WEBAPPVER_MINOR);
  if (prevGUI != currentGUI) {
    LOG_INFO("Web UI version mismatch detected. Cleaning up assets...");
    
    // --- Step 1: Notify User ---
    LoadingBoard* loadingBoard = (LoadingBoard*)displayManager.getSystemBoard(SystemBoardId::SYS_BOOT_LOADING);
    loadingBoard->setProgress("Cleaning up following upgrade", 45);
    displayManager.showBoard(loadingBoard);
    
    // --- Step 2: Remove Obsolete Assets ---
    LittleFS.remove(F("/index_d.htm"));
    LittleFS.remove(F("/index.htm"));
    LittleFS.remove(F("/keys.htm"));
    LittleFS.remove(F("/nrelogo.webp"));
    LittleFS.remove(F("/tfllogo.webp"));
    LittleFS.remove(F("/btlogo.webp"));
    LittleFS.remove(F("/tube.webp"));
    LittleFS.remove(F("/nr.webp"));
    LittleFS.remove(F("/favicon.svg"));
    LittleFS.remove(F("/favicon.png"));
    
    // Update local version record
    f = LittleFS.open(F("/webver"), "w");
    if (f) {
        f.println(currentGUI);
        f.close();
    }
  }
}

/**
 * @brief Implements the iConfigurable interface.
 */
void OtaUpdater::reapplyConfig(const Config& config) {
    updatesEnabled = config.firmwareUpdatesEnabled;
    dailyCheckEnabled = config.dailyUpdateCheckEnabled;
}
