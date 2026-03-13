/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/boards/tflBoard/src/tflBoard.cpp
 * Description: Implementation of TfLBoard using widgets and iDataSource.
 */

#include "tflBoard.hpp"
#include <Logger.hpp>

extern const char tflAttribution[];
extern const uint8_t Underground10[];

TfLBoard::TfLBoard() 
    : headWidget(0, 0, 256, 14), 
      servicesWidget(0, 15, 256, 26), 
      msgWidget(0, 56, 256, 8),
      lastUpdate(0) {
    headWidget.getClock().setFont(Underground10);
    headWidget.getClock().setBlink(false);
    tflAppkey[0] = '\0';
    tubeId[0] = '\0';
    tubeName[0] = '\0';

    // Configure service list columns for TfL
    ColumnDef cols[3] = {
        {60, 0},   // Line name
        {140, 0},  // Destination
        {56, 2}    // Expected time
    };
    servicesWidget.setColumns(3, cols);
}

void TfLBoard::onActivate() {
    dataSource.configure(tubeId, tflAppkey);
    headWidget.setTitle(tubeName);
    msgWidget.setMessage(tflAttribution);
    lastUpdate = 0;
}

void TfLBoard::onDeactivate() {}

void TfLBoard::tick(uint32_t ms) {
    if (ms - lastUpdate > 30000 || lastUpdate == 0) {
        int status = dataSource.updateData();
        lastUpdate = ms;

        if (status == 0) { // UPD_SUCCESS
            // In a real app, we'd cycle through disruption messages in the msgWidget
            stnMessages* msgs = dataSource.getMessagesData();
            if (msgs->numMessages > 0) {
                msgWidget.setMessage(msgs->messages[0]);
            } else {
                msgWidget.setMessage(tflAttribution);
            }
            
            // Populate widget data only when data actually changes
            TflStation* data = dataSource.getStationData();
            servicesWidget.clearRows();
            if (data->numServices > 0) {
                for (int i = 0; i < data->numServices; i++) {
                    const char* rowData[3] = {
                        data->service[i].lineName,
                        data->service[i].destination,
                        data->service[i].expectedTime
                    };
                    servicesWidget.addRow(rowData);
                }
            }
        }
    }

    headWidget.tick(ms);
    msgWidget.tick(ms);
    servicesWidget.tick(ms);
}

void TfLBoard::render(U8G2& display) {
    headWidget.render(display);
    
    TflStation* data = dataSource.getStationData();
    
    if (data->numServices > 0) {
        servicesWidget.render(display);
    } else {
        display.setFont(Underground10);
        display.drawStr(10, 35, "No arrivals scheduled.");
    }

    msgWidget.render(display);
}

void TfLBoard::renderAnimationUpdate(U8G2& display, uint32_t currentMillis) {
    headWidget.renderAnimationUpdate(display, currentMillis);
    msgWidget.renderAnimationUpdate(display, currentMillis);
    servicesWidget.renderAnimationUpdate(display, currentMillis);
}
