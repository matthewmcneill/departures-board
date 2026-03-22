/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/boards/tflBoard/layouts/layoutDefault.cpp
 * Description: Implementation of the default TfL Tube layout.
 */

#include "layoutDefault.hpp"
#include <displayManager.hpp>

extern const uint8_t Underground10[];

/**
 * @brief Default TfL layout constructor.
 * @param context Pointer to the application context.
 */
layoutTflDefault::layoutTflDefault(appContext* context) 
    : iTflLayout(context) {
    
    // --- Step 1: Widget Mapping ---
    // Configure widget positions exactly as they were in TfLBoard v2.5
    headWidget.setCoords(0, 0, 256, 12);
    headWidget.getClock().setFont(Underground10);
    headWidget.getClock().setBlink(true);

    servicesWidget.setCoords(0, 12, 256, 39);
    
    msgWidget.setCoords(0, 52, 256, 12);

    // --- Step 2: Column Layout ---
    // Configure service list columns for TfL (Underground specific)
    ColumnDef cols[3] = {
        {60, 0},   // Line name
        {140, 0},  // Destination
        {56, 2}    // Expected time
    };
    servicesWidget.setColumns(3, cols);
}
