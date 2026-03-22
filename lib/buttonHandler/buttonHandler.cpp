/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/systemManager/input/buttonHandler.cpp
 * Description: Implementation of the TTP223 touch sensor signal processing.
 *
 * Exported Functions/Classes:
 * - buttonHandler::update: Executes logic for tracking pulse timing.
 */

#include "buttonHandler.hpp"

buttonHandler::buttonHandler(uint8_t gpioPin) 
    : pin(gpioPin), lastState(false), currentState(false),
      shortTapFlag(false), longTapFlag(false),
      lastDebounceMs(0), touchStartedMs(0), lastTapTime(0),
      debounceDelay(50), longTapMs(1000) {
    pinMode(pin, INPUT);
}

void buttonHandler::update() {
    bool rawState = digitalRead(pin);

    // If the switch changed (due to noise or pressing)
    if (rawState != lastState) {
        lastDebounceMs = millis(); 
    }

    if ((millis() - lastDebounceMs) > debounceDelay) {
        // State has been stable for longer than debounce delay
        if (rawState != currentState) {
            currentState = rawState;

            if (currentState == HIGH) {
                // Button was just pressed
                touchStartedMs = millis();
                shortTapFlag = false;
                longTapFlag = false;
            } else {
                // Button was gently released
                unsigned long touchLengthMs = millis() - touchStartedMs;
                if (touchLengthMs < longTapMs) {
                    shortTapFlag = true;
                    lastTapTime = millis();
                } else {
                    longTapFlag = true;
                    lastTapTime = millis();
                }
            }
        }
    }
    lastState = rawState;
}

bool buttonHandler::isPressed() {
    return currentState;
}

bool buttonHandler::wasShortTapped() {
    if (shortTapFlag) {
        shortTapFlag = false;
        return true;
    }
    return false;
}

bool buttonHandler::wasLongTapped() {
    if (longTapFlag) {
        longTapFlag = false;
        return true;
    }
    return false;
}

void buttonHandler::setLongTapTime(unsigned long ms) {
    longTapMs = ms;
}

void buttonHandler::setDebounceTime(unsigned long ms) {
    debounceDelay = ms;
}
