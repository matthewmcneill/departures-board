#include <appContext.hpp>
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
#include <logger.hpp>

extern const char nrAttributionn[];
extern const uint8_t NatRailTall12[];
extern const uint8_t NatRailSmall9[];

NationalRailBoard::NationalRailBoard(appContext* contextPtr) 
    : context(contextPtr),
      headWidget(0, 0, 256, 14), 
      servicesWidget(0, 28, 256, 26), // Secondary services start lower
      msgWidget(0, 28, 256, 9, NatRailSmall9),      // Line 2 scroller
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
    LOG_INFO("DISPLAY", "NR Board: onActivate() called.");
    
    // Defer data source initialization if WiFi is not connected to avoid boot blocking
    if (WiFi.status() == WL_CONNECTED) {
        int status = dataSource.init("lite.realtime.nationalrail.co.uk", "/OpenLDBWS/wsdl.aspx?ver=2021-11-01", raildataYieldWrapper); 
        if (status != 0) {
            LOG_WARN("DISPLAY", "NR Board: dataSource.init() failed with status: " + String(status));
        } else {
            LOG_INFO("DISPLAY", "NR Board: dataSource.init() succeeded.");
        }
    } else {
        LOG_INFO("DISPLAY", "NR Board: WiFi not connected. Deferring dataSource.init().");
    }
    
    // Use the stored configuration
    dataSource.configure(nrToken, crsCode, platformFilter, callingCrsCode, nrTimeOffset);
    
    // Configure message pools
    if (context) {
        msgWidget.addMessagePool(&context->getGlobalMessagePool());
    }
    msgWidget.addMessagePool(dataSource.getMessagesData());
    
    // Set initial text (fallback)
    msgWidget.setText(nrAttributionn);
    lastUpdate = 0;
}

void NationalRailBoard::onDeactivate() {}

void NationalRailBoard::configure(const BoardConfig& config) {
    this->config = config;
    if (context) {
        ApiKey* key = context->getConfigManager().getKeyById(config.apiKeyId);
        if (key) setNrToken(key->token);
        else nrToken[0] = '\0';
    }
    setCrsCode(config.id);
    setPlatformFilter(config.filter);
    setCallingCrsCode(config.secondaryId);
    setCallingStation(config.secondaryName);
    setNrTimeOffset(config.timeOffset);
    setStationLat(config.lat);
    setStationLon(config.lon);
}

void NationalRailBoard::tick(uint32_t ms) {
    if (ms > nextViaToggle) {
        viaToggle = !viaToggle;
        nextViaToggle = ms + 4000;
    }

    headWidget.tick(ms);
    msgWidget.tick(ms);
    servicesWidget.tick(ms);
}

int NationalRailBoard::updateData() {
    if (!config.complete) {
        LOG_WARN("DISPLAY", "NR Board: Skipping updateData() - Configuration incomplete.");
        return 7; // Use a dedicated "Unconfigured" code if defined, or just return an error
    }

    // Check if we need to perform deferred initialization
    if (WiFi.status() == WL_CONNECTED && !dataSource.isInitialized()) {
        LOG_INFO("DISPLAY", "NR Board: Performing deferred dataSource.init()...");
        dataSource.init("lite.realtime.nationalrail.co.uk", "/OpenLDBWS/wsdl.aspx?ver=2021-11-01", raildataYieldWrapper);
    }

    LOG_INFO("DISPLAY", "NR Board: Starting data update...");
    lastUpdateStatus = dataSource.updateData();
    if (lastUpdateStatus != 0) {
        LOG_WARN("DISPLAY", "NR Board: Data update failed with status: " + String(lastUpdateStatus));
    } else {
        LOG_INFO("DISPLAY", "NR Board: Data update finished successfully.");
    }
    
    if (lastUpdateStatus == 0) { // UPD_SUCCESS
        // Update header once we have the station name
        NationalRailStation* data = dataSource.getStationData();
        if (data->location[0]) {
            headWidget.setTitle(data->location);
            headWidget.setCallingPoint(callingStation);
            headWidget.setPlatform(platformFilter);
            headWidget.setTimeOffset(nrTimeOffset);
            headWidget.setShowDate(true);
        }

        // Trigger scrolling of calling points if they changed
        if (data->numServices > 0 && data->service[0].calling[0]) {
            msgWidget.setText(data->service[0].calling);
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
    return lastUpdateStatus;
}

void NationalRailBoard::render(U8G2& display) {
    if (context && context->getsystemManager().isWifiPersistentError()) {
        iDisplayBoard* wifiError = context->getDisplayManager().getSystemBoard(SystemBoardId::SYS_ERROR_WIFI);
        if (wifiError) {
            wifiError->render(display);
            return;
        }
    }

    if (!config.complete) {
        // Delegate to system help boards
        SystemBoardId helpId = (config.errorType == 1) ? SystemBoardId::SYS_HELP_KEYS : SystemBoardId::SYS_HELP_CRS;
        if (context) {
            iDisplayBoard* help = context->getDisplayManager().getSystemBoard(helpId);
            if (help) help->render(display);
        }
        return;
    }

    headWidget.render(display);
    
    if (lastUpdateStatus == 5) { // UPD_UNAUTHORISED
        display.setFont(NatRailTall12);
        display.drawStr(5, 30, "API Token Invalid");
        display.setFont(NatRailSmall9);
        display.drawStr(5, 45, "Check portal settings.");
        return;
    }

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
    if (context && context->getsystemManager().isWifiPersistentError()) return;
    if (!config.complete) return;
    headWidget.renderAnimationUpdate(display, currentMillis);
    msgWidget.renderAnimationUpdate(display, currentMillis);
    servicesWidget.renderAnimationUpdate(display, currentMillis);
}
