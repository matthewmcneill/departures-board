/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/widgets/scrollingMessagePoolWidget.hpp
 * Description: Controller widget that orchestrates the rotation of text strings 
 *              sourced from multiple MessagePool instances. Inherits marquee 
 *              rendering from scrollingTextWidget and implements sequential 
 *              pool traversal.
 *
 * Exported Functions/Classes:
 * - scrollingMessagePoolWidget: UI component for managed message rotation.
 *   - addMessagePool(): Register a data source to the rotation.
 *   - tick(): Automated pool traversal and scroller logic.
 *   - loadNextMessage(): Internal selection logic.
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
    scrollingMessagePoolWidget(int _x, int _y, int _w, int _h, const uint8_t* _font = nullptr);
    virtual ~scrollingMessagePoolWidget() = default;

    /**
     * @brief Adds a message pool to the rotation.
     * @param pool Pointer to a managed MessagePool instance.
     */
    void addMessagePool(MessagePool* pool);

    /**
     * @brief Clears all registered message pools.
     */
    void clearPools();

    /**
     * @brief Overrides tick to handle pool transitions when a scroll finishes.
     */
    virtual void tick(uint32_t currentMillis) override;
};

#endif // SCROLLING_MESSAGE_POOL_WIDGET_HPP
