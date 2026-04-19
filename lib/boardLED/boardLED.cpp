/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/boardLED/boardLED.cpp
 * Description: Implementation for the board LED utility functions.
 *
 * Exported Functions/Classes:
 * - See boardLED.hpp
 */

#include "boardLED.hpp"

/**
 * @brief Initializes the physical GPIO pin defined by BOARD_LED.
 */
void BoardLED::init() {
  if (BOARD_LED >= 0) {
    pinMode(BOARD_LED, OUTPUT);
  }
}

/**
 * @brief Turns on the LED.
 */
void BoardLED::on() {
  if (BOARD_LED >= 0) {
    digitalWrite(BOARD_LED, BOARD_LED_ON);
  }
}

/**
 * @brief Turns off the LED.
 */
void BoardLED::off() {
  if (BOARD_LED >= 0) {
    digitalWrite(BOARD_LED, (!BOARD_LED_ON));
  }
}

/**
 * @brief Reads the current LED state and inverts it.
 */
void BoardLED::flip() {
  if (BOARD_LED >= 0) {
    digitalWrite(BOARD_LED, !digitalRead(BOARD_LED));
  }
}

/**
 * @brief Blinks the LED sequentially (OFF -> delay -> ON) blocking the thread briefly.
 * @param delay_ms Time in milliseconds to drop/blink the LED.
 */
void BoardLED::blink(uint32_t delay_ms) {
  if (BOARD_LED >= 0) {
    // --- Step 1: Invert the LED state to give visual feedback of processing ---
    BoardLED::flip();
    
    // --- Step 2: Block thread for visibility ---
    delay(delay_ms);
    
    // --- Step 3: Restore to original state ---
    BoardLED::flip();
  }
}
