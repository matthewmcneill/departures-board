/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/boardLED/boardLED.hpp
 * Description: Encapsulates the global hardware pin definitions for the diagnostic/status LED.
 * Provides a clean interface to initialize, turn on, turn off, and blink the board LED 
 * regardless of the physical Active-High or Active-Low wiring.
 *
 * Exported Functions/Classes:
 * - BoardLED: Static utility class for manipulating the physical diagnostic LED on the microcontroller.
 *   - init(): Initializes the physical GPIO pin.
 *   - on(): Asserts the LED to a lit state.
 *   - off(): Asserts the LED to an unlit state.
 *   - blink(): Performs a blocking toggle-delay-toggle sequence for bootloader stages.
 */

#pragma once

#include <Arduino.h>

/* --- Board LED Logic Configuration --- */
// 1. Resolve macro override logic
#ifndef BOARD_LED
  #ifdef LED_BUILTIN
    #define BOARD_LED LED_BUILTIN
  #else
    #define BOARD_LED 2 // Safe generic fallback
  #endif
#endif

// 2. Resolve Active-High or Active-Low macro definition
#ifndef BOARD_LED_ON
  #define BOARD_LED_ON HIGH
#endif

class BoardLED {
public:
  /**
   * @brief Initializes the physical GPIO pin defined by BOARD_LED.
   */
  static void init();

  /**
   * @brief Turns on the LED.
   */
  static void on();

  /**
   * @brief Turns off the LED.
   */
  static void off();

  /**
   * @brief Reads the current LED state and inverts it.
   */
  static void flip();

  /**
   * @brief Blinks the LED sequentially (OFF -> delay -> ON) blocking the thread briefly.
   * @param delay_ms Time in milliseconds to drop/blink the LED.
   */
  static void blink(uint32_t delay_ms);
};
