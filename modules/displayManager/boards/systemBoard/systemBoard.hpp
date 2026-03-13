/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/boards/systemBoard/systemBoard.hpp
 * Description: Groups standalone full-screen overlays (splash, setup wizards,
 *              various error screens, and screensavers).
 *
 * Exported Functions/Classes:
 * - getSystemBoardRegistry: Retrieve a configured system board singleton.
 * - drawStartupHeading: Main logo and version header.
 * - softResetBoard: Re-initialise system state without reboot.
 */

#pragma once

#include <U8g2lib.h>
#include "../../widgets/drawingPrimitives.hpp"
#include <WiFi.h>
#include <time.h>
#include "../../../../../include/gfx/xbmgfx.h"
#include <IPAddress.h>
#include <WiFiManager.h>
#include <displayManager.hpp>

class WiFiManager;

extern int VERSION_MAJOR;
extern int VERSION_MINOR;



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
