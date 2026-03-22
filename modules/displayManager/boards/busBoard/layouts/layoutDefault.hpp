/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/boards/busBoard/layouts/layoutDefault.hpp
 * Description: Default layout for Bus, providing a pixel-perfect 
 *              recreation of the legacy v2.5 board.
 */

#ifndef BUS_LAYOUT_DEFAULT_HPP
#define BUS_LAYOUT_DEFAULT_HPP

#include "../iBusLayout.hpp"

/**
 * @brief Default Bus layout.
 */
class layoutBusDefault : public iBusLayout {
public:
    layoutBusDefault(appContext* context);
};

#endif // BUS_LAYOUT_DEFAULT_HPP
