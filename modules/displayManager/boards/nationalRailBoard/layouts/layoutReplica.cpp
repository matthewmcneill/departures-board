/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * Module: modules/displayManager/boards/nationalRailBoard/views/viewReplica.cpp
 * Description: Implementation of an alternative National Rail layout.
 */

#ifndef NATRAIL_REPLICA_CPP
#define NATRAIL_REPLICA_CPP

#include "layoutReplica.hpp"
#include <displayManager.hpp>

extern const uint8_t NatRailTall12[];
extern const uint8_t NatRailSmall9[];

layoutNrReplica::layoutNrReplica(appContext* context) 
    : iNationalRailLayout(context) {
    
    // DIFFERENT LAYOUT: Move the message bar to the TOP
    msgWidget.setCoords(0, 0, 200, 12);
    msgWidget.setFont(NatRailSmall9);

    // MOVE THE CLOCK to the top right
    sysClock.setCoords(200, 0, 56, 12);
    sysClock.setVisible(true);

    // ROW 0 immediately under the message bar
    row0Time.setCoords(0, 12, 60, 14);
    row0Time.setFont(NatRailTall12);
    
    row0Dest.setCoords(60, 12, 196, 14);
    row0Dest.setFont(NatRailTall12);

    // EXPANDED SERVICES
    servicesWidget.setCoords(0, 26, 256, 38);
    
    // HIDE THE HEADER (maybe we don't need station name here)
    headWidget.setVisible(false);

    noDataLabel.setCoords(10, 40, 256, 15);
    noDataLabel.setFont(NatRailTall12);
    noDataLabel.setText("No services found.");

    // Configure service list columns (Different alignment or widths)
    ColumnDef cols[4] = {
        {20, 0},   // No ordinal? Just 1, 2, 3
        {35, 0},   // Scheduled time
        {25, 2},   // Platform (Right aligned)
        {176, 0}   // Destination
    };
    servicesWidget.setColumns(4, cols);
}

#endif
