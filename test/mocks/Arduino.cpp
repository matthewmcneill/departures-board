/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: tools/layoutsim/src/Arduino.cpp
 * Description: Emscripten/WASM mock stub for the Arduino framework.
 */

#include "Arduino.h"
MockSPI SPI;
MockWire Wire;

#include "WiFi.h"
MockWiFi WiFi;

#include "LittleFS.h"
LittleFSFS LittleFS;

#include "appContext.hpp"
class appContext appContext;

#include "displayManager.hpp"
DisplayManager displayManager;

const char* weekdays[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
