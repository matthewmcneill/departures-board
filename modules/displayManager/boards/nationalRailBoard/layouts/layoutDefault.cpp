/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * Module: modules/displayManager/boards/nationalRailBoard/views/viewDefault.cpp
 * Description: Implementation of the default National Rail layout logic.
 */

#include "layoutDefault.hpp"
#include <displayManager.hpp>

extern const uint8_t NatRailTall12[];
extern const uint8_t NatRailSmall9[];

layoutNrDefault::layoutNrDefault(appContext* context) 
    : iNationalRailLayout(context), viaToggle(false), nextViaToggle(0) {
    
    // Configure widget positions exactly as they were in NationalRailBoard v2.5
    headWidget.setCoords(0, 0, 256, 12);

    // Row 0 - The primary service
    row0Time.setCoords(0, 12, 60, 14);
    row0Time.setFont(NatRailTall12);
    
    row0Dest.setCoords(60, 12, 196, 14);
    row0Dest.setFont(NatRailTall12);

    // Secondary services
    servicesWidget.setCoords(0, 26, 256, 39);
    
    // Bottom message
    msgWidget.setCoords(0, 52, 256, 12);
    msgWidget.setFont(NatRailSmall9);

    // Disable bits that didn't exist in the v2.5 layout
    sysClock.setVisible(false);

    // Configure service list columns
    ColumnDef cols[4] = {
        {23, 0},   // Ordinal (2nd, 3rd)
        {27, 0},   // Scheduled time
        {16, 1},   // Platform
        {190, 0}   // Destination
    };
    servicesWidget.setColumns(4, cols);
}

void layoutNrDefault::tick(uint32_t currentMillis) {
    // Call base class to tick all widgets
    iNationalRailLayout::tick(currentMillis);
}

void layoutNrDefault::render(U8G2& display) {
    iNationalRailLayout::render(display);
}
