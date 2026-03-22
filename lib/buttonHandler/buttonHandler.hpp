/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/systemManager/input/buttonHandler.hpp
 * Description: TTP223 Touch Sensor hardware implementation of the iButton interface.
 *
 * Exported Functions/Classes:
 * - buttonHandler: Concrete class handling polling and debouncing of a digital pin.
 *   - update(): Read IO values and update debounce states.
 *   - isPressed(): Raw polling status.
 *   - wasShortTapped(): Returns true if short release sequence completed.
 *   - wasLongTapped(): Returns true if long hold release sequence completed.
 */
#pragma once

#include <Arduino.h>

/**
 * @brief Hardware implementation for a momentary switch or touch sensor.
 */
class buttonHandler {
private:
    uint8_t pin; ///< The GPIO pin connected to the sensor

    bool lastState; ///< The raw state during the last poll
    bool currentState; ///< The debounced active state
    bool shortTapFlag; ///< True if a short tap was detected
    bool longTapFlag; ///< True if a long tap was detected

    unsigned long lastDebounceMs; ///< Timestamp for debounce logic
    unsigned long touchStartedMs; ///< Timestamp when touch was first active
    unsigned long lastTapTime; ///< Timestamp of the last completed tap

    unsigned long debounceDelay; ///< Milliseconds to wait for signal to settle
    unsigned long longTapMs; ///< Milliseconds required to trigger a long tap

public:
    /**
     * @brief Construct a new touch sensor driver.
     * @param gpioPin The hardware pin the sensor is wired to.
     */
    buttonHandler(uint8_t gpioPin);

    // Button state interface
    void update();
    bool isPressed();
    bool wasShortTapped();
    bool wasLongTapped();

    /**
     * @brief Configure the duration required for a long press.
     * @param ms Duration in milliseconds.
     */
    void setLongTapTime(unsigned long ms);

    /**
     * @brief Configure the debounce filter threshold.
     * @param ms Duration in milliseconds.
     */
    void setDebounceTime(unsigned long ms);
};
