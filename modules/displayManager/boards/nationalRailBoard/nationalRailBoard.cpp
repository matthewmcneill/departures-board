/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/boards/nationalRailBoard/src/nationalRailBoard.cpp
 * Description: Implementation of NationalRailBoard using widgets and iDataSource.
 */

#include "nationalRailBoard.hpp"
#include <Logger.hpp>

extern const char nrAttributionn[];
extern const uint8_t NatRailTall12[];
extern const uint8_t NatRailSmall9[];

NationalRailBoard::NationalRailBoard() 
    : headWidget(0, 0, 256, 14), 
      servicesWidget(0, 28, 256, 26), // Secondary services start lower
      msgWidget(0, 28, 256, 9),      // Line 2 scroller
      lastUpdate(0), 
      viaToggle(false),
      nextViaToggle(0) {
    nrToken[0] = '\0';
    crsCode[0] = '\0';
    callingCrsCode[0] = '\0';
    callingStation[0] = '\0';
    platformFilter[0] = '\0';
    nrTimeOffset = 0;
    stationLat = 0;
    stationLon = 0;
    altStationEnabled = false;
    altStarts = 0;
    altEnds = 0;
    altCrsCode[0] = '\0';
    altLat = 0;
    altLon = 0;
    altCallingCrsCode[0] = '\0';
    altCallingStation[0] = '\0';
    altPlatformFilter[0] = '\0';
    altStationActive = false;

    // Configure secondary service list columns
    ColumnDef cols[4] = {
        {23, 0},   // Ordinal (2nd, 3rd)
        {27, 0},   // Scheduled time
        {16, 1},   // Platform
        {190, 0}   // Destination... actually we'd need more logic here
    };
    servicesWidget.setColumns(4, cols);
}

void NationalRailBoard::onActivate() {
    dataSource.init("online.gadec.uk", "/darwin/wsdl.xml", nullptr); // Simplified init
    dataSource.configure(nrToken, crsCode, platformFilter, callingCrsCode, nrTimeOffset);
    lastUpdate = 0;
}

void NationalRailBoard::onDeactivate() {}

void NationalRailBoard::tick(uint32_t ms) {
    if (ms - lastUpdate > 60000 || lastUpdate == 0) {
        int status = dataSource.updateData();
        lastUpdate = ms;
        
        if (status == 0) { // UPD_SUCCESS
            // Update header once we have the station name
            NationalRailStation* data = dataSource.getStationData();
            if (data->location[0]) {
                headWidget.setTitle(data->location);
                headWidget.setCallingPoint(callingStation);
                headWidget.setPlatform(platformFilter);
                headWidget.setTimeOffset(nrTimeOffset);
                headWidget.setShowDate(true);
            }

            // Setup message scroller with calling points or service messages
            if (data->numServices > 0) {
                msgWidget.setMessage(data->service[0].calling); // Simple version for now
            } else {
                msgWidget.setMessage(nrAttributionn);
            }
            
            // Populate services list for drawing later
            servicesWidget.clearRows();
            if (data->numServices > 1) {
                for (int i = 1; i < data->numServices; i++) {
                    if (i - 1 >= 16) break;
                    sprintf(cachedOrdinals[i-1], "%d%s", i+1, (i==1?"nd":(i==2?"rd":"th")));
                    const char* rowData[4] = {
                        cachedOrdinals[i-1],
                        data->service[i].sTime,
                        data->service[i].platform,
                        data->service[i].destination
                    };
                    servicesWidget.addRow(rowData);
                }
            }
        }
    }

    if (ms > nextViaToggle) {
        viaToggle = !viaToggle;
        nextViaToggle = ms + 4000;
    }

    headWidget.tick(ms);
    msgWidget.tick(ms);
    servicesWidget.tick(ms);
}

void NationalRailBoard::render(U8G2& display) {
    headWidget.render(display);
    
    NationalRailStation* data = dataSource.getStationData();
    if (data->numServices > 0) {
        // Draw Primary Service (Row 0)
        display.setFont(NatRailTall12);
        display.drawStr(0, 25, data->service[0].sTime);
        
        const char* dest = viaToggle && data->service[0].via[0] ? data->service[0].via : data->service[0].destination;
        display.drawStr(60, 25, dest);

        // Draw Line 2 (Messages)
        msgWidget.render(display);

        // Draw Secondary Services (Row 1+)
        if (data->numServices > 1) {
            servicesWidget.render(display);
        }
    } else {
        display.setFont(NatRailTall12);
        display.drawStr(10, 40, "No services found.");
    }
}

void NationalRailBoard::renderAnimationUpdate(U8G2& display, uint32_t currentMillis) {
    headWidget.renderAnimationUpdate(display, currentMillis);
    msgWidget.renderAnimationUpdate(display, currentMillis);
    servicesWidget.renderAnimationUpdate(display, currentMillis);
}
