/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/boards/systemBoard/include/systemBoard.hpp
 * Description: Groups standalone full-screen overlays (splash, setup wizards,
 *              various error screens, and screensavers).
 *
 * Provides:
 * - Boot screens (showSplashScreen, progressBar)
 * - Error screens (showNoDataScreen, showTokenErrorScreen)
 * - Wifi and Setup Wizard Screens (showSetupScreen)
 * - Hardware reset and reboot functions.
 */

#pragma once

#include <U8g2lib.h>
#include <drawingPrimitives.hpp>
#include <WiFi.h>
#include <WiFiManager.h>
#include <WiFiConfig.hpp>
#include <time.h>
#include "../../../../include/gfx/xbmgfx.h"

extern U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI u8g2;
extern int VERSION_MAJOR;
extern int VERSION_MINOR;
extern bool sleepClock;

/**
 * @brief Show setup keys help screen
 */
void showSetupKeysHelpScreen();

/**
 * @brief Show setup CRS help screen
 */
void showSetupCrsHelpScreen();

/**
 * @brief Render the initial configuration wizard instructions via WiFi
 */
void showSetupScreen(IPAddress ip);

/**
 * @brief Render an error screen when no data could be fetched from the API
 */
void showNoDataScreen();

/**
 * @brief Render a critical error if National Rail WSDL fails to init
 */
void showWsdlFailureScreen();

/**
 * @brief Render an error meaning the National Rail or TfL token is invalid
 */
void showTokenErrorScreen();

/**
 * @brief Render a CRS error screen
 */
void showCRSErrorScreen();

/**
 * @brief Draw the firmware compile build date and time at the bottom right
 */
void drawBuildTime();

/**
 * @brief Draw the main Departures Board intro logo and heading
 */
void drawStartupHeading();

/**
 * @brief Render the screensaver / sleep mode UI (moving clock or logo)
 */
void drawSleepingScreen();

/**
 * @brief Show or hide the OTA update available icon
 * @param show true to show, false to clear
 */
void showFirmwareUpdateIcon(bool show);

/**
 * @brief Show firmware update warning screen
 * @param msg The version name fetched from Github
 * @param secs Seconds until auto-reboot and flash
 */
void showFirmwareUpdateWarningScreen(const char *msg, int secs);

/**
 * @brief Show firmware update progress bar during flash memory write
 * @param percent 0-100 indicating download progress
 */
void showFirmwareUpdateProgress(int percent);

/**
 * @brief Show update complete screen
 */
void showFirmwareUpdateCompleteScreen(const char *title, const char *msg1, const char *msg2, const char *msg3, int secs, bool showReboot);

String getBuildTime();
void doClockCheck();
bool isAltActive();
void raildataCallback(int stage, int nServices);
void updateMyUrl();
bool setAlternateStation();
void updateRssFeed();
void softResetBoard();
void wmConfigModeCallback(WiFiManager *myWiFiManager);

void addRssMessage();
void removeRssMessage();
void tflCallback();

// --- Network & Boot State ---
bool getWifiConfigured();
void setWifiConfigured(bool configured);

bool getWifiConnected();
void setWifiConnected(bool connected);

unsigned long getLastWiFiReconnect();
void setLastWiFiReconnect(unsigned long time);

bool getFirstLoad();
void setFirstLoad(bool load);

int getPrevProgressBarPosition();
void setPrevProgressBarPosition(int pos);

int getStartupProgressPercent();
void setStartupProgressPercent(int percent);
