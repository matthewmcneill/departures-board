#include <appContext.hpp>
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
#include <logger.hpp>

extern const char tflAttribution[];
extern const uint8_t Underground10[];

TfLBoard::TfLBoard(appContext* contextPtr) 
    : context(contextPtr),
      headWidget(0, 0, 256, 14), 
      servicesWidget(0, 15, 256, 26), 
      msgWidget(0, 56, 256, 8, NatRailSmall9),
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
    dataSource.configure(tubeId, tflAppkey, yieldCallbackWrapper);
    headWidget.setTitle(tubeName);
    
    // Configure message pools
    if (context) {
        msgWidget.addMessagePool(&context->getGlobalMessagePool());
    }
    msgWidget.addMessagePool(dataSource.getMessagesData());
    
    // Set initial text (fallback)
    msgWidget.setText(tflAttribution);
    lastUpdate = 0;
}

void TfLBoard::onDeactivate() {}

void TfLBoard::configure(const BoardConfig& config) {
    this->config = config;
    if (context) {
        ApiKey* key = context->getConfigManager().getKeyById(config.apiKeyId);
        if (key) setTflAppkey(key->token);
        else tflAppkey[0] = '\0';
    }
    setTubeId(config.id);
    setTubeName(config.name);
}

void TfLBoard::tick(uint32_t ms) {
    headWidget.tick(ms);
    msgWidget.tick(ms);
    servicesWidget.tick(ms);
}

int TfLBoard::updateData() {
    if (!config.complete) {
        LOG_WARN("DISPLAY", "TfL Board: Skipping updateData() - Configuration incomplete.");
        return 7;
    }
    int status = dataSource.updateData();
    lastUpdateStatus = status;

    if (status == 0) { // UPD_SUCCESS
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
    return status;
}

void TfLBoard::render(U8G2& display) {
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
        display.setFont(Underground10);
        display.drawStr(10, 35, "API Token Invalid");
        display.setFont(NatRailSmall9);
        display.drawStr(10, 50, "Check portal settings.");
        return;
    }

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
    if (context && context->getsystemManager().isWifiPersistentError()) return;
    if (!config.complete) return;
    headWidget.renderAnimationUpdate(display, currentMillis);
    msgWidget.renderAnimationUpdate(display, currentMillis);
    servicesWidget.renderAnimationUpdate(display, currentMillis);
}
