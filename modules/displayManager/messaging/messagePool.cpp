/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/messaging/messagePool.cpp
 * Description: Implementation of memory-efficient string message storage.
 *
 * Exported Functions/Classes:
 * - MessagePool: [Class implementation]
 *   - addMessage: Memory-safe message ingestion with capacity enforcement.
 *   - getMessage: Safe index-based accessor.
 *   - getCount: Reports current queue depth.
 *   - clear: Complete cache purge/resource recovery.
 */

#include "messagePool.hpp"

/**
 * @brief Construct a new Message Pool.
 * @param maxItems The maximum number of messages this pool should retain.
 */
MessagePool::MessagePool(size_t maxItems) : maxMessages(maxItems) {
    messages.reserve(maxItems);
}

/**
 * @brief Adds a new message to the pool. Discards oldest if full.
 * @param msg The null-terminated message string to add.
 */
void MessagePool::addMessage(const char* msg) {
    if (msg == nullptr || msg[0] == '\0') return;

    // --- Step 1: Enforce Bounds ---
    // If the pool is at capacity, remove the oldest (first) message.
    if (messages.size() >= maxMessages) {
        messages.erase(messages.begin());
    }
    
    // --- Step 2: Ingest ---
    messages.push_back(String(msg));
}

/**
 * @brief Retrieves a message at a specific index.
 * @param index The zero-based index of the message.
 * @return const char* Pointer to the message string, or nullptr if out of bounds.
 */
const char* MessagePool::getMessage(size_t index) const {
    if (index < messages.size()) {
        return messages[index].c_str();
    }
    return nullptr;
}

/**
 * @brief Gets the current number of retained messages.
 * @return size_t Count of messages.
 */
size_t MessagePool::getCount() const {
    return messages.size();
}

/**
 * @brief Drop the most recently added message from the collection.
 */
void MessagePool::removeLastMessage() {
    if (!messages.empty()) {
        messages.pop_back();
    }
}

/**
 * @brief Empties the message pool, recovering memory.
 */
void MessagePool::clear() {
    messages.clear();
}
