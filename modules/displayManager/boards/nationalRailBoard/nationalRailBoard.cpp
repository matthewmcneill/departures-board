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
      activeLayout(nullptr),
      lastUpdate(0), 
      viaToggle(false),
      nextViaToggle(0) {
    
    // Instantiate default view
    activeLayout = new layoutNrDefault(context);

    nrToken[0] = '\0';
    crsCode[0] = '\0';
    callingCrsCode[0] = '\0';
    callingStation[0] = '\0';
    platformFilter[0] = '\0';
    nrTimeOffset = 0;
    stationLat = 0;
    stationLon = 0;
}

NationalRailBoard::~NationalRailBoard() {
    if (activeLayout) delete activeLayout;
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
    if (context && activeLayout) {
        activeLayout->msgWidget.addMessagePool(&context->getGlobalMessagePool());
        activeLayout->msgWidget.addMessagePool(dataSource.getMessagesData());
        
        // Set initial text (fallback)
        activeLayout->msgWidget.setText(nrAttributionn);
    }
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
        
        // Push the update to the view immediately
        NationalRailStation* data = dataSource.getStationData();
        if (data->numServices > 0 && activeLayout) {
            const char* dest = viaToggle && data->service[0].via[0] ? data->service[0].via : data->service[0].destination;
            activeLayout->row0Dest.setText(dest);
        }
    }

    if (activeLayout) activeLayout->tick(ms);
}

int NationalRailBoard::updateData() {
    if (lastUpdateStatus == UPD_PENDING) {
        lastUpdateStatus = dataSource.getLastUpdateStatus();
        if (lastUpdateStatus == UPD_PENDING) {
            return UPD_PENDING;
        }
    } else {
        if (!config.complete) {
            LOG_WARN("DISPLAY", "NR Board: Skipping updateData() - Configuration incomplete.");
            return 7; // Use a dedicated "Unconfigured" code if defined
        }

        if (WiFi.status() == WL_CONNECTED && !dataSource.isInitialized()) {
            LOG_INFO("DISPLAY", "NR Board: Performing deferred dataSource.init()...");
            dataSource.init("lite.realtime.nationalrail.co.uk", "/OpenLDBWS/wsdl.aspx?ver=2021-11-01", nullptr);
        }

        LOG_INFO("DISPLAY", "NR Board: Starting data update...");
        lastUpdateStatus = dataSource.updateData();
        if (lastUpdateStatus == UPD_PENDING) {
            return UPD_PENDING;
        }
    }

    if (lastUpdateStatus != 0 && lastUpdateStatus != 1) {
        LOG_WARN("DISPLAY", "NR Board: Data update failed with status: " + String(lastUpdateStatus));
        if (lastUpdateStatus > 2) {
            consecutiveErrors++;
        }
    } else {
        LOG_INFO("DISPLAY", "NR Board: Data update finished successfully.");
        consecutiveErrors = 0;
    }
    
    if (lastUpdateStatus == 0) { // UPD_SUCCESS
        // Update header once we have the station name
        NationalRailStation* data = dataSource.getStationData();
        if (data->location[0] && activeLayout) {
            activeLayout->headWidget.setTitle(data->location);
            activeLayout->headWidget.setCallingPoint(callingStation);
            activeLayout->headWidget.setPlatform(platformFilter);
            activeLayout->headWidget.setTimeOffset(nrTimeOffset);
            activeLayout->headWidget.setShowDate(true);
        }

        if (data->numServices > 0 && activeLayout) {
            // Push Primary Service (Row 0)
            activeLayout->row0Time.setText(data->service[0].sTime);
            
            const char* dest = viaToggle && data->service[0].via[0] ? data->service[0].via : data->service[0].destination;
            activeLayout->row0Dest.setText(dest);

            // Trigger scrolling of calling points if they changed
            if (data->service[0].calling[0]) {
                activeLayout->msgWidget.setText(data->service[0].calling);
            }
        }
        
        // Populate services list (Secondary)
        if (activeLayout) {
            activeLayout->servicesWidget.clearRows();
            if (data->numServices > 1) { // Start from second service
                for (int i = 1; i < data->numServices; i++) {
                    if (i >= 17) break; // Array limit
                    sprintf(cachedOrdinals[i-1], "%d%s", i+1, (i==1?"nd":(i==2?"rd":"th")));
                    const char* rowData[4] = {
                        cachedOrdinals[i-1],
                        data->service[i].sTime,
                        data->service[i].platform,
                        data->service[i].destination
                    };
                    activeLayout->servicesWidget.addRow(rowData);
                }
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
        if (context) {
            iDisplayBoard* help = context->getDisplayManager().getSystemBoard(SystemBoardId::SYS_SETUP_HELP);
            if (help) help->render(display);
        }
        return;
    }

    if (lastUpdateStatus > 2 && consecutiveErrors >= 3) {
        if (context) {
            SystemBoardId id = context->getDisplayManager().mapErrorToId(lastUpdateStatus);
            iDisplayBoard* errBoard = context->getDisplayManager().getSystemBoard(id);
            if (errBoard) {
                errBoard->render(display);
                return;
            }
        }
    }

    if (activeLayout) {
        NationalRailStation* data = dataSource.getStationData();
        if (data->numServices > 0) {
            activeLayout->render(display);
        } else {
            activeLayout->headWidget.render(display);
            display.setFont(NatRailTall12);
            display.drawStr(10, 40, "No services found.");
        }
    }
}

void NationalRailBoard::renderAnimationUpdate(U8G2& display, uint32_t currentMillis) {
    if (context && context->getsystemManager().isWifiPersistentError()) return;
    if (!config.complete) return;
    if (lastUpdateStatus > 2 && consecutiveErrors >= 3) {
        if (context) {
            SystemBoardId id = context->getDisplayManager().mapErrorToId(lastUpdateStatus);
            iDisplayBoard* errBoard = context->getDisplayManager().getSystemBoard(id);
            if (errBoard) {
                errBoard->renderAnimationUpdate(display, currentMillis);
                return;
            }
        }
    }

    if (activeLayout) activeLayout->renderAnimationUpdate(display, currentMillis);
}
