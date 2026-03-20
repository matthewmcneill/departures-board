/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * Module: modules/displayManager/boards/busBoard/views/viewDefault.cpp
 * Description: Implementation of the default Bus layout logic.
 */

#include "layoutDefault.hpp"
#include <displayManager.hpp>

extern const uint8_t Underground10[];

layoutBusDefault::layoutBusDefault(appContext* context) 
    : iBusLayout(context) {
    
    // Configure widget positions exactly as they were in BusBoard v2.5
    headWidget.setCoords(0, 0, 256, 12);
    headWidget.getClock().setFont(Underground10);
    headWidget.getClock().setBlink(true);

    servicesWidget.setCoords(0, 12, 256, 39);
    
    msgWidget.setCoords(0, 52, 256, 12);

    // Configure service list columns for Bus layout
    ColumnDef cols[3] = {
        {25, 0},   // Route number
        {180, 0},  // Destination
        {51, 2}    // Time
    };
    servicesWidget.setColumns(3, cols);
}
