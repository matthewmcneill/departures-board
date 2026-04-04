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
 *
 * Exported Functions/Classes:
 * - scrollingMessagePoolWidget: [Class implementation]
 *   - addMessagePool(): Registers a data source.
 *   - clearPools(): Resets all tracked pools and indices.
 *   - tick(): Automated marquee management and pool traversal.
 */

#include "scrollingMessagePoolWidget.hpp"

/**
 * @brief Initialize the message pool widget.
 * @param _x X coordinate.
 * @param _y Y coordinate.
 * @param _w Width.
 * @param _h Height.
 * @param _font Optional font override.
 */
scrollingMessagePoolWidget::scrollingMessagePoolWidget(int _x, int _y, int _w, int _h, const uint8_t* _font)
    : scrollingTextWidget(_x, _y, _w, _h, _font), currentPoolIndex(0), currentMessageIndex(0), showingInterleavedNext(true) {
    interleavedMessage[0] = '\0';
}

/**
 * @brief Register a MessagePool as a data source for the rotation.
 * @param pool Managed pointer to an existing pool.
 */
void scrollingMessagePoolWidget::addMessagePool(MessagePool* pool) {
    if (pool != nullptr) {
        pools.push_back(pool);
    }
}

/**
 * @brief Clears all registered message pools.
 */
void scrollingMessagePoolWidget::clearPools() {
    pools.clear();
    currentPoolIndex = 0;
    currentMessageIndex = 0;
    showingInterleavedNext = true;
    interleavedMessage[0] = '\0';
    resetScroll();
    setText("");
}

void scrollingMessagePoolWidget::setInterleavedMessage(const char* msg) {
    if (msg) {
        strlcpy(interleavedMessage, msg, sizeof(interleavedMessage));
    } else {
        interleavedMessage[0] = '\0';
    }
}

/**
 * @brief Searches through injected pools for the next available string.
 */
bool scrollingMessagePoolWidget::loadNextMessage() {
    if (interleavedMessage[0] != '\0' && showingInterleavedNext) {
        setText(interleavedMessage);
        showingInterleavedNext = false;
        return true;
    }

    if (pools.empty()) {
        if (interleavedMessage[0] != '\0') {
            setText(interleavedMessage);
            showingInterleavedNext = false;
            return true;
        }
        return false;
    }

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
                    showingInterleavedNext = true;
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

    // If all pools are completely empty, gracefully fall back
    if (interleavedMessage[0] != '\0') {
        setText(interleavedMessage);
        showingInterleavedNext = false;
        return true;
    }

    return false;
}

/**
 * @brief Ticks the underlying scroller and handles transitions.
 * Automatically loads the next message when the current one finishes.
 * @param currentMillis System runtime in milliseconds.
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
