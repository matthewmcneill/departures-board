/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/boards/busBoard/src/busBoard.cpp
 * Description: Implementation of BusBoard using widgets and iDataSource.
 *
 * Exported Functions/Classes:
 * - BusBoard::BusBoard: Constructor.
 * - BusBoard::onActivate: Called when board activates.
 * - BusBoard::onDeactivate: Called when board deactivates.
 * - BusBoard::tick: Periodic update handler.
 * - BusBoard::render: Full rendering handler.
 * - BusBoard::renderAnimationUpdate: Partial rendering for animations.
 */

#include "busBoard.hpp"
#include <Logger.hpp>

extern const char btAttribution[]; ///< Global attribution text
extern const uint8_t Underground10[]; ///< Built-in font

/**
 * @brief Constructs the BusBoard and configures widget positions.
 */
BusBoard::BusBoard() 
    : headWidget(0, 0, 256, 14), 
      servicesWidget(0, 15, 256, 39),
      msgWidget(0, 56, 256, 8),
      lastUpdate(0), 
      needsRefresh(true) {
    headWidget.getClock().setFont(Underground10);
    headWidget.getClock().setBlink(false);
    busAtco[0] = '\0';
    busName[0] = '\0';
    busFilter[0] = '\0';
    enableBus = false;

    // --- Step 1: Configure widget constraints ---
    // Configure service list columns for Bus layout
    ColumnDef cols[3] = {
        {25, 0},   // Route number (left aligned)
        {180, 0},  // Destination (left aligned)
        {51, 2}    // Time (right aligned)
    };
    servicesWidget.setColumns(3, cols);
}

/**
 * @brief Called when the display board is visually activated.
 */
void BusBoard::onActivate() {
    dataSource.configure(busAtco, busFilter);
    headWidget.setTitle(busName);
    msgWidget.setMessage(btAttribution);
    lastUpdate = 0; // Trigger immediate update
}

/**
 * @brief Called when the display board is deactivated.
 */
void BusBoard::onDeactivate() {
    // Cleanup if needed
}

/**
 * @brief Updates internal logic and periodically fetches new data.
 * @param ms Current system epoch in milliseconds.
 */
void BusBoard::tick(uint32_t ms) {
    // --- Step 1: Check update constraints ---
    // Attempt an update every 30000 ms natively, or if no update occurred
    if (ms - lastUpdate > 30000 || lastUpdate == 0) {
        int status = dataSource.updateData();
        lastUpdate = ms;
        needsRefresh = true;
        
        // --- Step 2: Handle incoming data ---
        // Populate widget data only when data updates and has actually changed
        if (status == 0) { // UPD_SUCCESS
            BusStop* data = dataSource.getStationData();
            servicesWidget.clearRows();
            if (data->numServices > 0) {
                for (int i = 0; i < data->numServices; i++) {
                    const char* rowData[3] = {
                        data->service[i].routeNumber,
                        data->service[i].destination,
                        data->service[i].expectedTime
                    };
                    servicesWidget.addRow(rowData);
                }
            }
        }
    }

    // --- Step 3: Propagate tick to nested widgets ---
    headWidget.tick(ms);
    msgWidget.tick(ms);
    servicesWidget.tick(ms);
    
    // In a real implementation, we'd also handle the rotation of services/messages here
    // for the 3rd line, but for this first pass we'll keep it simple.
}

/**
 * @brief Perform a full screen render to the U8G2 context.
 * @param display Primary graphics context.
 */
void BusBoard::render(U8G2& display) {
    headWidget.render(display);
    
    // Render the departures
    BusStop* data = dataSource.getStationData();
    
    if (data->numServices > 0) {
        servicesWidget.render(display);
    } else {
        display.setFont(Underground10);
        display.drawStr(10, 35, "No scheduled services.");
    }

    msgWidget.render(display);
}

/**
 * @brief Partially updates screen content to process micro-animations.
 * @param display Primary graphics context.
 * @param currentMillis Current time offset.
 */
void BusBoard::renderAnimationUpdate(U8G2& display, uint32_t currentMillis) {
    headWidget.renderAnimationUpdate(display, currentMillis);
    msgWidget.renderAnimationUpdate(display, currentMillis);
    servicesWidget.renderAnimationUpdate(display, currentMillis);
}
