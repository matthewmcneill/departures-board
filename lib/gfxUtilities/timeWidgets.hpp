/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/gfxUtilities/timeWidgets.hpp
 * Description: Contains specific header rendering widgets for the various dashboard types
 *              (i.e. Analog National Rail Clock vs Digital TfL Time).
 */

#pragma once

#include <U8g2lib.h>
#include <Arduino.h>
#include "drawingPrimitives.hpp"
#include <time.h>

extern U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI u8g2;
extern struct tm timeinfo;
extern const char* weekdays[];
extern int nrTimeOffset;
extern bool dateEnabled;
extern bool clockEnabled;

/**
 * @brief Draw the National Rail header row including station name and clock
 * @param stopName Primary station name
 * @param callingStopName Filter/via station name
 * @param platFilter Active platform filter
 * @param timeOffset Time offset applied to board
 */
void drawCurrentTime(const char* stopName, const char* callingStopName, const char* platFilter, int timeOffset);

/**
 * @brief Render the current time in the top right corner for TfL/Bus boards
 * @param update If true, visually refresh the display area immediately
 */
void drawCurrentTimeUG(bool update);
