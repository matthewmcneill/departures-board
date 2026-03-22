/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: src/departuresBoard.hpp
 * Description: Centralized build-time configuration and hardware defaults.
 *              Allows for board-level tuning via platformio.ini flags.
 *
 * Exported Functions/Classes:
 * - MAX_BOARDS: Maximum number of configurable display boards.
 * - DATAUPDATEINTERVAL: Default data refresh interval (ms).
 * - FASTDATAUPDATEINTERVAL: Accelerated refresh for urgent updates.
 * - UGDATAUPDATEINTERVAL: TfL Underground refresh rate.
 * - BUSDATAUPDATEINTERVAL: Bus data refresh rate.
 * - SCREENSAVERINTERVAL: Cycling interval for snooze screens (ms).
 * - MAX_CONFIG_CONSUMERS: Maximum classes subscribing to config updates.
 * - MAX_BUS_ATCO_HISTORY: Maximum ATCO history depth.
 */

#pragma once

// --- Versioning ---
#define VERSION_MAJOR 3
#define VERSION_MINOR 0

// --- Hardware Constraints ---
#ifndef MAX_BOARDS
#define MAX_BOARDS 6
#endif

// --- Timing & Intervals (ms) ---
#ifndef DATAUPDATEINTERVAL
#define DATAUPDATEINTERVAL 150000 // Default data refresh interval
#endif

#ifndef FASTDATAUPDATEINTERVAL
#define FASTDATAUPDATEINTERVAL 45000 // Accelerated refresh for urgent updates
#endif

#ifndef UGDATAUPDATEINTERVAL
#define UGDATAUPDATEINTERVAL 30000 // TfL Underground refresh rate
#endif

#ifndef BUSDATAUPDATEINTERVAL
#define BUSDATAUPDATEINTERVAL 45000 // Bus data refresh rate
#endif

#ifndef SCREENSAVERINTERVAL
#define SCREENSAVERINTERVAL 10000 // Cycling interval for snooze screens
#endif

// --- Persistence Limits ---
#ifndef MAX_CONFIG_CONSUMERS
#define MAX_CONFIG_CONSUMERS 10
#endif

// --- Hardware Pins ---
#ifndef BUTTON_PIN
#define BUTTON_PIN 34 // Touch sensor input pin (Defined in platformio.ini per env)
#endif

#ifndef MAX_BUS_ATCO_HISTORY
#define MAX_BUS_ATCO_HISTORY 10
#endif
