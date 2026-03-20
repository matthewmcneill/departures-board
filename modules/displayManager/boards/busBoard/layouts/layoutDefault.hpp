/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * Module: modules/displayManager/boards/busBoard/layouts/layoutDefault.hpp
 * Description: Default layout for Bus, migrating hardcoded board logic.
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
