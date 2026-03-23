/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: tools/layoutsim/src/displayManager.hpp
 * Description: Mock graphics manager exposing dimensional constraints to the layout simulator.
 *
 * Exported Functions/Classes:
 * - DisplayManager: Mock hardware context class
 *   - yieldAnimationUpdate(): Stubs deep firmware yield requests
 */

#ifndef DISPLAY_MANAGER_MOCK_HPP
#define DISPLAY_MANAGER_MOCK_HPP

#include <U8g2lib.h>

class DisplayManager {
public:
    /**
     * @brief Stubs deep firmware yield requests preventing thread blocks
     */
    void yieldAnimationUpdate() {}
};

extern DisplayManager displayManager; // Global mock display instance

// Mock constants usually found in display headers
#define SCREEN_WIDTH 256 // Defined pixel width limit of physical panels
#define SCREEN_HEIGHT 64 // Defined pixel height limit of physical panels

#endif
