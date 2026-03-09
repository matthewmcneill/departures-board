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
 */

#include "otaUpdater.hpp"
#include <timeWidgets.hpp>
#include <HTTPClient.h>
#include <HTTPUpdateGitHub.h>
#include <githubClient.h>

extern github ghUpdate;

OtaUpdater ota;

OtaUpdater::OtaUpdater() {
    firmwareUpdatesEnabled = false;
    dailyUpdateCheckEnabled = false;
    fwUpdateCheckTimer = 0;
    prevUpdateCheckDay = -1;
}

void OtaUpdater::configure(bool firmwareUpdates, bool dailyUpdateCheck) {
    firmwareUpdatesEnabled = firmwareUpdates;
    dailyUpdateCheckEnabled = dailyUpdateCheck;
}

bool OtaUpdater::getFirmwareUpdatesEnabled() const { return firmwareUpdatesEnabled; }
void OtaUpdater::setFirmwareUpdatesEnabled(bool enabled) { firmwareUpdatesEnabled = enabled; }

bool OtaUpdater::getDailyUpdateCheckEnabled() const { return dailyUpdateCheckEnabled; }
void OtaUpdater::setDailyUpdateCheckEnabled(bool enabled) { dailyUpdateCheckEnabled = enabled; }

void OtaUpdater::tick() {
    if (dailyUpdateCheckEnabled && millis() > fwUpdateCheckTimer) {
        fwUpdateCheckTimer = millis() + 3300000 + random(600000); // check again in 55 to 65 mins
        
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
  showFirmwareUpdateProgress((cur*100)/total);
}


/**
 * @brief Check GitHub if a newer firmware than our VERSION_MAJOR and VERSION_MINOR exists
 */
bool isFirmwareUpdateAvailable() {
  int releaseMajor = ghUpdate.releaseId.substring(1,ghUpdate.releaseId.indexOf(".")).toInt();
  int releaseMinor = ghUpdate.releaseId.substring(ghUpdate.releaseId.indexOf(".")+1,ghUpdate.releaseId.indexOf("-")).toInt();
  if (VERSION_MAJOR > releaseMajor) return false;
  if ((VERSION_MAJOR == releaseMajor) && (VERSION_MINOR >= releaseMinor)) return false;
  return true;
}

/**
 * @brief Attempts to install newer firmware if available and reboots
 */
bool OtaUpdater::checkForFirmwareUpdate() {
  bool result = true;

  if (!isFirmwareUpdateAvailable()) return result;

  // Find the firmware binary in the release assets
  String updatePath="";
  for (int i=0;i<ghUpdate.releaseAssets;i++){
    if (ghUpdate.releaseAssetName[i] == "firmware.bin") {
      updatePath = ghUpdate.releaseAssetURL[i];
      break;
    }
  }
  if (updatePath.length()==0) {
    //  No firmware binary in release assets
    return result;
  }

  unsigned long tmr=millis()+1000;
  for (int i=30;i>=0;i--) {
    showFirmwareUpdateWarningScreen(ghUpdate.releaseDescription.c_str(),i);
    while (tmr>millis()) {
      yield();
    }
    tmr=millis()+1000;
  }
  u8g2.clearBuffer();
  showFirmwareUpdateProgress(0);  // So we don't have a blank screen
  WiFiClientSecure client;
  client.setInsecure();
  httpUpdate.onProgress(update_progress);
  httpUpdate.rebootOnUpdate(false); // Don't auto reboot, we'll handle it

  HTTPUpdateResult ret = httpUpdate.handleUpdate(client, updatePath, ghUpdate.accessToken);
  const char* msgTitle = "Firmware Update";
  switch (ret) {
    case HTTP_UPDATE_FAILED:
      char msg[60];
      sprintf(msg,"The update failed with error %d.",httpUpdate.getLastError());
      result=false;
      for (int i=20;i>=0;i--) {
        showFirmwareUpdateCompleteScreen(msgTitle,msg,httpUpdate.getLastErrorString().c_str(),"",i,false);
        delay(1000);
      }
      break;

    case HTTP_UPDATE_NO_UPDATES:
      for (int i=10;i>=0;i--) {
        showFirmwareUpdateCompleteScreen(msgTitle,"","No firmware updates were available.","",i,false);
        delay(1000);
      }
      break;

    case HTTP_UPDATE_OK:
      for (int i=20;i>=0;i--) {
        showFirmwareUpdateCompleteScreen(msgTitle,"The firmware update has completed successfully.","For more information visit the URL below:","github.com/gadec-uk/departures-board/releases",i,true);
        delay(1000);
      }
      ESP.restart();
      break;
  }
  u8g2.clearBuffer();
  drawStartupHeading();
  u8g2.sendBuffer();
  return result;
}
