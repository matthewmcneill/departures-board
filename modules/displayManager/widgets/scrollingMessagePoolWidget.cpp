/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/widgets/scrollingMessagePoolWidget.cpp
 * Description: Implementation of the message pool rotation logic.
 */

#include "scrollingMessagePoolWidget.hpp"

/**
 * @brief Construct a new scrolling message pool widget.
 */
scrollingMessagePoolWidget::scrollingMessagePoolWidget(int _x, int _y, int _w, int _h, const uint8_t* _font)
    : scrollingTextWidget(_x, _y, _w, _h, _font), currentPoolIndex(0), currentMessageIndex(0) {
}

/**
 * @brief Attach a message pool to this widget's lifecycle.
 */
void scrollingMessagePoolWidget::addMessagePool(MessagePool* pool) {
    if (pool != nullptr) {
        pools.push_back(pool);
    }
}

/**
 * @brief Searches through injected pools for the next available string.
 */
bool scrollingMessagePoolWidget::loadNextMessage() {
    if (pools.empty()) return false;

    size_t poolsChecked = 0;
    while (poolsChecked < pools.size()) {
        // --- Step 1: Select Active Pool ---
        MessagePool* p = pools[currentPoolIndex];
        
        // --- Step 2: Validate Message Availability ---
        if (p->getCount() > 0) {
            if (currentMessageIndex < p->getCount()) {
                const char* msg = p->getMessage(currentMessageIndex);
                if (msg) {
                    // Load into the marquee engine
                    setText(msg);
                    currentMessageIndex++;
                    return true;
                }
            }
        }

        // --- Step 3: Pool Traversal ---
        // If current pool is exhausted or empty, move to the next.
        currentPoolIndex = (currentPoolIndex + 1) % pools.size();
        currentMessageIndex = 0;
        poolsChecked++;
    }

    return false;
}

/**
 * @brief Ticks the underlying scroller and handles transitions.
 */
void scrollingMessagePoolWidget::tick(uint32_t currentMillis) {
    if (!isVisible) return;
    
    // If we have no text currently, try to load one
    if (currentText[0] == '\0') {
        if (!loadNextMessage()) return;
    }

    // Call base class to handle pixel movement
    scrollingTextWidget::tick(currentMillis);

    // If base class signaled it's done with the current string, load next
    if (isScrollFinished()) {
        if (loadNextMessage()) {
            resetScroll();
        }
    }
}
