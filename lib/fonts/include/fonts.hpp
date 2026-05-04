/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/fonts/include/fonts.hpp
 * Description: Contains declarations for custom binary font arrays used by the OLED display
 *              to replicate authentic UK railway and TfL station boards.
 * 
 * NOTE ON ARCHITECTURE:
 * To comply with the One Definition Rule (ODR) and prevent linker "multiple definition"
 * errors when included across varying UI modules, this header now 
 * strictly contains 'extern' declarations. The massive byte arrays are defined exactly 
 * once in src/fonts.cpp, ensuring they are compiled into Flash memory only once.
 */

#pragma once
#include <U8g2lib.h>

extern const uint8_t NatRailSmall9[];
extern const uint8_t NatRailTall12[];
extern const uint8_t NatRailClockSmall7[];
extern const uint8_t NatRailClockLarge9[];
extern const uint8_t Underground10[];
extern const uint8_t UndergroundClock8[];
extern const uint8_t WeatherIcons16[]; // 16x16 weather condition symbols
extern const uint8_t WeatherIcons11[]; // 11x11 weather condition symbols
extern const uint8_t WifiIcons11[];    // 11x11 WiFi signal strength and status icons
extern const uint8_t SWRClockHuge11[]; // 11px high SWR Clock font
extern const uint8_t SWRClockMega13[]; // 13px high SWR Platform font
extern const uint8_t FormationIcons7[]; // 7px high Train Formation icons
