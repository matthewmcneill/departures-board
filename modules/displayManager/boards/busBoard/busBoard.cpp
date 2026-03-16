#include <appContext.hpp>
/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
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
#include <logger.hpp>

extern const char btAttribution[]; ///< Global attribution text
extern const uint8_t Underground10[]; ///< Built-in font
extern const uint8_t NatRailSmall9[]; ///< Small font for scrolling messages

/**
 * @brief Constructs the BusBoard and configures widget positions.
 */
BusBoard::BusBoard(appContext* contextPtr) 
    : context(contextPtr),
      headWidget(0, 0, 256, 14), 
      servicesWidget(0, 15, 256, 39),
      msgWidget(0, 56, 256, 8, NatRailSmall9),
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
    
    // Configure message pools
    if (context) {
        msgWidget.addMessagePool(&context->getGlobalMessagePool());
    }
    msgWidget.addMessagePool(dataSource.getMessagesData());
    
    // Set initial text (fallback)
    msgWidget.setText(btAttribution);
    lastUpdate = 0; // Trigger immediate update
}

/**
 * @brief Called when the display board is deactivated.
 */
void BusBoard::onDeactivate() {
    // Cleanup if needed
}

void BusBoard::configure(const BoardConfig& config) {
    this->config = config;
    setBusAtco(config.id);
    setBusName(config.name);
    setBusLat(config.lat);
    setBusLon(config.lon);
    setBusFilter(config.filter);
}

/**
 * @brief Updates internal logic and periodically fetches new data.
 * @param ms Current system epoch in milliseconds.
 */
void BusBoard::tick(uint32_t ms) {
    headWidget.tick(ms);
    msgWidget.tick(ms);
    servicesWidget.tick(ms);
}

int BusBoard::updateData() {
    if (!config.complete) {
        LOG_WARN("DISPLAY", "Bus Board: Skipping updateData() - Configuration incomplete.");
        return 7;
    }
    int status = dataSource.updateData();
    lastUpdateStatus = status;
    needsRefresh = true;
    
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
    return status;
}

/**
 * @brief Perform a full screen render to the U8G2 context.
 * @param display Primary graphics context.
 */
void BusBoard::render(U8G2& display) {
    if (context && context->getsystemManager().isWifiPersistentError()) {
        iDisplayBoard* wifiError = context->getDisplayManager().getSystemBoard(SystemBoardId::SYS_ERROR_WIFI);
        if (wifiError) {
            wifiError->render(display);
            return;
        }
    }

    if (!config.complete) {
        // Delegate to system help boards
        // Bus boards only have "missing ID" state in current logic, as they don't have a unique key.
        SystemBoardId helpId = SystemBoardId::SYS_HELP_CRS; // Reuse CRS help for generic ID help
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
    if (context && context->getsystemManager().isWifiPersistentError()) return;
    if (!config.complete) return;
    headWidget.renderAnimationUpdate(display, currentMillis);
    msgWidget.renderAnimationUpdate(display, currentMillis);
    servicesWidget.renderAnimationUpdate(display, currentMillis);
}
