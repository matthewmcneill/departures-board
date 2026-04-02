/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/otaUpdater/otaUpdater.cpp
 * Description: Implementation of firmware lifecycle and OTA update sequence.
 *
 * Exported Functions/Classes:
 * - otaUpdater: [Class implementation]
 *   - tick: Main lifecycle check loop.
 *   - checkForFirmwareUpdate: Executes the upgrade process.
 *   - checkPostWebUpgrade: Syncs local filesystem resources after updates.
 */

#include "otaUpdater.hpp"
#include <HTTPClient.h>
#include <hTTPUpdateGitHub.hpp>
#include <githubClient.hpp>
#include <time.h>
#include <logger.hpp>
#include <configManager.hpp>
#include <memory>
#include <appContext.hpp>
#include <departuresBoard.hpp>
#include <timeManager.hpp>

otaUpdater ota;                       // Global OTA maintenance instance singleton
github ghUpdate("gadec-uk/departures-board", ""); // Global GitHub client for updates

otaUpdater::otaUpdater() : context(nullptr), prevUpdateCheckDay(-1), fwUpdateCheckTimer(0), updatesEnabled(false), dailyCheckEnabled(false) {
}

void otaUpdater::tick() {
    if (dailyCheckEnabled && millis() > fwUpdateCheckTimer) {
        fwUpdateCheckTimer = millis() + 3300000 + random(600000); 
        
        context->getTimeManager().updateCurrentTime();
        const struct tm& timeinfo = context->getTimeManager().getCurrentTime();
        
        if (timeinfo.tm_mday != prevUpdateCheckDay) {
            if (ghUpdate.getLatestRelease()) {
                checkForFirmwareUpdate();
                prevUpdateCheckDay = timeinfo.tm_mday;
            }
        }
    }
}

void update_progress(int cur, int total) {
  int percent = (cur*100)/total;
  if (ota.onProgress) {
      ota.onProgress(percent);
  }
}

/**
 * @brief Semantic version comparison against latest GitHub release metadata.
 * @return true if remote version is strictly newer than local build.
 */
bool isFirmwareUpdateAvailable() {
  int releaseMajor = ghUpdate.releaseId.substring(1,ghUpdate.releaseId.indexOf(".")).toInt();
  int releaseMinor = ghUpdate.releaseId.substring(ghUpdate.releaseId.indexOf(".")+1,ghUpdate.releaseId.indexOf("-")).toInt();
  if (VERSION_MAJOR > releaseMajor) return false;
  if ((VERSION_MAJOR == releaseMajor) && (VERSION_MINOR >= releaseMinor)) return false;
  return true;
}

/**
 * @brief Manual check and execution of the firmware update process.
 * Downloads the binary from GitHub assets and performs a system reboot on success.
 * @return true if the process was successfully initiated.
 */
bool otaUpdater::checkForFirmwareUpdate() {
  bool result = true;
  if (!isFirmwareUpdateAvailable()) return result;

  String updatePath="";
  for (int i=0;i<ghUpdate.releaseAssets;i++){
    if (ghUpdate.releaseAssetName[i] == "firmware.bin") {
      updatePath = ghUpdate.releaseAssetURL[i];
      break;
    }
  }
  if (updatePath.length()==0) return result;

  unsigned long tmr=millis()+1000;
  for (int i=30;i>=0;i--) {
    if (onWarning) {
        onWarning(ghUpdate.releaseDescription.c_str(), i);
    }
    while (tmr>millis()) {
      yield();
    }
    tmr=millis()+1000;
  }
  
  update_progress(0, 100);
  std::unique_ptr<WiFiClientSecure> client(new (std::nothrow) WiFiClientSecure());
  if (!client) {
    LOG_ERROR("OTA", "Memory allocation failed for upgrade client!");
    return false;
  }
  client->setInsecure();
  httpUpdate.onProgress(update_progress);
  httpUpdate.rebootOnUpdate(false); 

  HTTPUpdateResult ret = httpUpdate.handleUpdate(*client, updatePath, ghUpdate.accessToken);
  
  switch (ret) {
    case HTTP_UPDATE_FAILED:
      result=false;
      for (int i=20;i>=0;i--) {
        if (onStateChange) onStateChange(FwUpdateState::FAILED, httpUpdate.getLastErrorString().c_str(), i);
        delay(1000);
      }
      break;

    case HTTP_UPDATE_NO_UPDATES:
      for (int i=10;i>=0;i--) {
        if (onStateChange) onStateChange(FwUpdateState::NO_UPDATES, "", i);
        delay(1000);
      }
      break;

    case HTTP_UPDATE_OK:
      for (int i=20;i>=0;i--) {
        if (onStateChange) onStateChange(FwUpdateState::SUCCESS, "", i);
        delay(1000);
      }
      ESP.restart();
      break;
  }
  
  return result;
}

void otaUpdater::checkPostWebUpgrade() {
  String prevGUI = "";
  File f = LittleFS.open(F("/webver"), "r");
  if (f) {
    prevGUI = f.readString();
    f.close();
  }
  prevGUI.trim();
  
  String currentGUI = String(VERSION_MAJOR) + F(".") + String(VERSION_MINOR);
  if (prevGUI != currentGUI) {
    // Logging implies we cleaned up
    
    if (onPostUpgradeProgress) {
        onPostUpgradeProgress("Cleaning up following upgrade", 45);
    }
    
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
    
    f = LittleFS.open(F("/webver"), "w");
    if (f) {
        f.println(currentGUI);
        f.close();
    }
  }
}

void otaUpdater::reapplyConfig(const Config& cfg) {
    updatesEnabled = cfg.firmwareUpdatesEnabled;
    dailyCheckEnabled = cfg.dailyUpdateCheckEnabled;
}
