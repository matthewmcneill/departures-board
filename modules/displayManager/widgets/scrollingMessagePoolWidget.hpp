/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/widgets/scrollingMessagePoolWidget.hpp
 * Description: Specialized widget that cycles through messages from multiple pools
 *              using the generic scrollingTextWidget base.
 *
 * Exported Functions/Classes:
 * - scrollingMessagePoolWidget: UI component for message rotation.
 */

#ifndef SCROLLING_MESSAGE_POOL_WIDGET_HPP
#define SCROLLING_MESSAGE_POOL_WIDGET_HPP

#include "scrollingTextWidget.hpp"
#include "../messaging/messagePool.hpp"
#include <vector>

/**
 * @brief Widget that monitors one or more MessagePools and feeds their 
 *        content sequentially into the scrolling engine.
 */
class scrollingMessagePoolWidget : public scrollingTextWidget {
private:
    std::vector<MessagePool*> pools;
    size_t currentPoolIndex;
    size_t currentMessageIndex;

    /**
     * @brief Internal logic to select the next string from the combined pools.
     * @return bool True if a new string was successfully loaded.
     */
    bool loadNextMessage();

public:
    /**
     * @brief Construct a new scrolling message pool widget.
     * @param _x X-coordinate.
     * @param _y Y-coordinate.
     * @param _w Width (0 for screen width).
     * @param _h Height.
     * @param _font Font array.
     */
    scrollingMessagePoolWidget(int _x, int _y, int _w, int _h, const uint8_t* _font);
    virtual ~scrollingMessagePoolWidget() = default;

    /**
     * @brief Adds a message pool to the rotation.
     * @param pool Pointer to a managed MessagePool instance.
     */
    void addMessagePool(MessagePool* pool);

    /**
     * @brief Overrides tick to handle pool transitions when a scroll finishes.
     */
    virtual void tick(uint32_t currentMillis) override;
};

#endif // SCROLLING_MESSAGE_POOL_WIDGET_HPP
