/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * Module: modules/displayManager/boards/tflBoard/views/viewDefault.cpp
 * Description: Implementation of the default TfL Tube layout logic.
 */

#include "layoutDefault.hpp"
#include <displayManager.hpp>

extern const uint8_t Underground10[];

layoutTflDefault::layoutTflDefault(appContext* context) 
    : iTflLayout(context) {
    
    // Configure widget positions exactly as they were in TfLBoard v2.5
    headWidget.setCoords(0, 0, 256, 12);
    headWidget.getClock().setFont(Underground10);
    headWidget.getClock().setBlink(true);

    servicesWidget.setCoords(0, 12, 256, 39);
    
    msgWidget.setCoords(0, 52, 256, 12);

    // Configure service list columns for TfL
    ColumnDef cols[3] = {
        {60, 0},   // Line name
        {140, 0},  // Destination
        {56, 2}    // Expected time
    };
    servicesWidget.setColumns(3, cols);
}
