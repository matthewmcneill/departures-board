/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/displayManager/displayManager.cpp
 * Description: Implementation of the DisplayManager singleton orchestrator, mapping carousel logic
 *              and rendering sleep screens safely.
 */

#include "displayManager.hpp"
#include <drawingPrimitives.hpp>

// Only defined once globally
DisplayManager displayManager;

DisplayManager::DisplayManager() : activeSlotIndex(0), sleepClock(true), isSleeping(false), forcedSleep(false), timer(0), brightness(50), sleepEnabled(false), sleepStarts(0), sleepEnds(6), flipScreen(false) {}

int DisplayManager::getBrightness() const { return brightness; }
void DisplayManager::setBrightness(int newBrightness) { 
    brightness = newBrightness; 
    u8g2.setContrast(brightness);
}

bool DisplayManager::getFlipScreen() const { return flipScreen; }
void DisplayManager::setFlipScreen(bool newFlipScreen) { 
    flipScreen = newFlipScreen; 
    u8g2.setFlipMode(flipScreen ? 1 : 0);
}

bool DisplayManager::getSleepEnabled() const { return sleepEnabled; }
void DisplayManager::setSleepEnabled(bool newSleepEnabled) { sleepEnabled = newSleepEnabled; }

byte DisplayManager::getSleepStarts() const { return sleepStarts; }
void DisplayManager::setSleepStarts(byte newSleepStarts) { sleepStarts = newSleepStarts; }

byte DisplayManager::getSleepEnds() const { return sleepEnds; }
void DisplayManager::setSleepEnds(byte newSleepEnds) { sleepEnds = newSleepEnds; }

bool DisplayManager::getForcedSleep() const { return forcedSleep; }
void DisplayManager::setForcedSleep(bool active) { forcedSleep = active; }

bool DisplayManager::getSleepClock() const { return sleepClock; }
void DisplayManager::setSleepClock(bool active) { sleepClock = active; }

bool DisplayManager::getIsSleeping() const { return isSleeping; }

void DisplayManager::resetState() {
    isSleeping = false;
    timer = 0;
}

bool DisplayManager::isSnoozing() {
    if (forcedSleep) return true;
    if (!sleepEnabled) return false;
    
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)) return false;
    
    byte myHour = timeinfo.tm_hour;
    if (sleepStarts > sleepEnds) {
        if ((myHour >= sleepStarts) || (myHour < sleepEnds)) return true; else return false;
    } else {
        if ((myHour >= sleepStarts) && (myHour < sleepEnds)) return true; else return false;
    }
}

void DisplayManager::drawSleepingScreen() {
    char sysTime[8];
    char sysDate[29];

    u8g2.setContrast(DIMMED_BRIGHTNESS);
    u8g2.clearBuffer();
    
    // Check if we should render sleep clock
    if (!sleepClock) {
        u8g2.sendBuffer();
        return;
    }
    
    struct tm timeinfo;
    getLocalTime(&timeinfo);
    sprintf(sysTime,"%02d:%02d",timeinfo.tm_hour,timeinfo.tm_min);
    strftime(sysDate,29,"%d %B %Y",&timeinfo);

    int offset = (getStringWidth(sysDate)-getStringWidth(sysTime))/2;
    u8g2.setFont(NatRailTall12);
    int y = random(39);
    int x = random(SCREEN_WIDTH-getStringWidth(sysDate));
    u8g2.drawStr(x+offset,y,sysTime);
    u8g2.setFont(NatRailSmall9);
    u8g2.drawStr(x,y+13,sysDate);
    
    u8g2.sendBuffer();
}

void DisplayManager::tick(unsigned long currentMillis) {
    bool wasSleeping = isSleeping;
    isSleeping = isSnoozing();

    if (isSleeping && currentMillis > timer) {
        // Handle screensaver transitions
        drawSleepingScreen();
        timer = currentMillis + SCREENSAVERINTERVAL;
        return;
    } else if (wasSleeping && !isSleeping) {
        // Clean exit
        u8g2.clearDisplay();
        // The individual board will handle redrawing on their next update cycle
    }
    
    // Otherwise render carousel ticks if we are not sleeping
    if (!isSleeping) {
        IStation* board = getActiveBoard();
        if (board != nullptr) {
            board->tick(currentMillis);
        }
    }
}

void DisplayManager::setBoardType(int slotIndex, BoardType type) {
    if (slotIndex < 0 || slotIndex >= MAX_BOARDS) return;
    
    switch (type) {
        case BoardType::NR_BOARD:
            slots[slotIndex] = NationalRailBoard();
            break;
        case BoardType::TFL_BOARD:
            slots[slotIndex] = TfLBoard();
            break;
        case BoardType::BUS_BOARD:
            slots[slotIndex] = BusBoard();
            break;
    }
}

void DisplayManager::clearSlot(int slotIndex) {
    if (slotIndex >= 0 && slotIndex < MAX_BOARDS) {
        slots[slotIndex] = std::monostate{};
    }
}

BoardVariant* DisplayManager::getSlot(int slotIndex) {
    if (slotIndex >= 0 && slotIndex < MAX_BOARDS) {
        return &slots[slotIndex];
    }
    return nullptr;
}

int DisplayManager::cycleNext() {
    int startingIndex = activeSlotIndex;
    do {
        activeSlotIndex = (activeSlotIndex + 1) % MAX_BOARDS;
        if (!std::holds_alternative<std::monostate>(slots[activeSlotIndex])) {
            return activeSlotIndex; // Found a valid board
        }
    } while (activeSlotIndex != startingIndex);
    
    return activeSlotIndex; // Fallback entirely, returning current index
}

IStation* DisplayManager::getActiveBoard() {
    if (std::holds_alternative<NationalRailBoard>(slots[activeSlotIndex])) {
        return &std::get<NationalRailBoard>(slots[activeSlotIndex]);
    } else if (std::holds_alternative<TfLBoard>(slots[activeSlotIndex])) {
        return &std::get<TfLBoard>(slots[activeSlotIndex]);
    } else if (std::holds_alternative<BusBoard>(slots[activeSlotIndex])) {
        return &std::get<BusBoard>(slots[activeSlotIndex]);
    }
    return nullptr; 
}

BoardType DisplayManager::getActiveBoardType() {
    if (std::holds_alternative<NationalRailBoard>(slots[activeSlotIndex])) return BoardType::NR_BOARD;
    if (std::holds_alternative<TfLBoard>(slots[activeSlotIndex])) return BoardType::TFL_BOARD;
    if (std::holds_alternative<BusBoard>(slots[activeSlotIndex])) return BoardType::BUS_BOARD;
    return BoardType::NR_BOARD; // fallback
}

int DisplayManager::getActiveSlotIndex() const {
    return activeSlotIndex;
}
