/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/boards/nationalRailBoard/layouts/layoutDefault.cpp
 * Description: Implementation of the default National Rail layout.
 */

#include "layoutDefault.hpp"
#include <displayManager.hpp>

extern const uint8_t NatRailTall12[];
extern const uint8_t NatRailSmall9[];

/**
 * @brief Default National Rail layout constructor.
 * @param context Pointer to the application context.
 */
layoutNrDefault::layoutNrDefault(appContext* context) 
    : iNationalRailLayout(context), viaToggle(false), nextViaToggle(0) {
    
    // --- Step 1: Widget Placement ---
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

    // --- Step 2: Specialized Configuration ---
    // Disable bits that didn't exist in the v2.5 layout
    sysClock.setVisible(false);

    noDataLabel.setCoords(10, 40, 256, 15);
    noDataLabel.setFont(NatRailTall12);
    noDataLabel.setText("No services found.");

    // Configure service list columns
    ColumnDef cols[4] = {
        {23, 0},   // Ordinal (2nd, 3rd)
        {27, 0},   // Scheduled time
        {16, 1},   // Platform
        {190, 0}   // Destination
    };
    servicesWidget.setColumns(4, cols);
}

/**
 * @brief Logic update for NR widgets.
 * @param currentMillis Current uptime in milliseconds.
 */
void layoutNrDefault::tick(uint32_t currentMillis) {
    // Call base class to tick all widgets
    iNationalRailLayout::tick(currentMillis);
}

/**
 * @brief Full frame render for the NR layout.
 * @param display Reference to U8g2.
 */
void layoutNrDefault::render(U8G2& display) {
    iNationalRailLayout::render(display);
}
