/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/messaging/messagePool.hpp
 * Description: Encapsulates the storage and retrieval of station or system messages,
 *              avoiding large fixed multi-dimensional arrays to save memory.
 *
 * Exported Functions/Classes:
 * - MessagePool: Container for dynamically storing and retrieving strings.
 */

#ifndef MESSAGE_POOL_HPP
#define MESSAGE_POOL_HPP

#include <Arduino.h>
#include <vector>

/**
 * @brief Class for managing a bounded pool of text messages.
 *        Used to store API service messages, RSS headlines, or local station alerts.
 */
class MessagePool {
private:
    std::vector<String> messages;
    size_t maxMessages;
    
public:
    /**
     * @brief Construct a new Message Pool.
     * @param maxItems The maximum number of messages this pool should retain.
     */
    MessagePool(size_t maxItems = 4);
    
    virtual ~MessagePool() = default;

    /**
     * @brief Adds a new message to the pool. Overwrites oldest if full.
     * @param msg The null-terminated message string to add.
     */
    void addMessage(const char* msg);

    /**
     * @brief Retrieves a message at a specific index.
     * @param index The zero-based index of the message.
     * @return const char* Pointer to the message string, or nullptr if out of bounds.
     */
    const char* getMessage(size_t index) const;

    /**
     * @brief Gets the current number of retained messages.
     * @return size_t Count of messages.
     */
    size_t getCount() const;

    /**
     * @brief Removes the most recently added message.
     */
    void removeLastMessage();

    /**
     * @brief Empties the message pool, recovering memory.
     */
    void clear();
};

#endif // MESSAGE_POOL_HPP
