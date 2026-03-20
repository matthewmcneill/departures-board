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
 * Module: lib/boards/tflBoard/src/tflBoard.cpp
 * Description: Implementation of TfLBoard using widgets and iDataSource.
 */

#include "tflBoard.hpp"
#include <logger.hpp>

extern const char tflAttribution[];
extern const uint8_t Underground10[];

TfLBoard::TfLBoard(appContext* contextPtr) 
    : context(contextPtr),
      activeLayout(nullptr),
      lastUpdate(0) {
    
    // Instantiate default view
    activeLayout = new layoutTflDefault(context);

    tflAppkey[0] = '\0';
    tubeId[0] = '\0';
    tubeName[0] = '\0';
}

TfLBoard::~TfLBoard() {
    if (activeLayout) delete activeLayout;
}

void TfLBoard::onActivate() {
    dataSource.configure(tubeId, tflAppkey, yieldCallbackWrapper);
    
    if (activeLayout) {
        activeLayout->headWidget.setTitle(tubeName);
        
        // Configure message pools
        if (context) {
            activeLayout->msgWidget.addMessagePool(&context->getGlobalMessagePool());
        }
        activeLayout->msgWidget.addMessagePool(dataSource.getMessagesData());
        
        // Set initial text (fallback)
        activeLayout->msgWidget.setText(tflAttribution);
    }
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
    if (strcmp(config.id, "HUBTCR") == 0) {
        setTubeId("940GZZLUTCR");
    } else {
        setTubeId(config.id);
    }
    setTubeName(config.name);
    dataSource.setFilter(config.filter);
}

void TfLBoard::tick(uint32_t ms) {
    if (activeLayout) activeLayout->tick(ms);
}

int TfLBoard::updateData() {
    if (lastUpdateStatus == UPD_PENDING) {
        lastUpdateStatus = dataSource.getLastUpdateStatus();
        if (lastUpdateStatus == UPD_PENDING) {
            return UPD_PENDING;
        }
    } else {
        if (!config.complete) {
            LOG_WARN("DISPLAY", "TfL Board: Skipping updateData() - Configuration incomplete.");
            return 7;
        }
        
        LOG_INFO("DISPLAY", "TfL Board: Starting data update...");
        lastUpdateStatus = dataSource.updateData();
        if (lastUpdateStatus == UPD_PENDING) {
            return UPD_PENDING;
        }
    }

    if (lastUpdateStatus != 0 && lastUpdateStatus != 1) {
        LOG_WARN("DISPLAY", "TfL Board: Data update failed with status: " + String(lastUpdateStatus));
        if (lastUpdateStatus > 2) {
            consecutiveErrors++;
        }
    } else {
        LOG_INFO("DISPLAY", "TfL Board: Data update finished successfully.");
        consecutiveErrors = 0;
    }

    if (lastUpdateStatus == 0) { // UPD_SUCCESS
        // Populate widget data
        TflStation* data = dataSource.getStationData();
        if (activeLayout) {
            activeLayout->servicesWidget.clearRows();
            if (data->numServices > 0) {
                for (int i = 0; i < data->numServices; i++) {
                    const char* rowData[3] = {
                        data->service[i].lineName,
                        data->service[i].destination,
                        data->service[i].expectedTime
                    };
                    activeLayout->servicesWidget.addRow(rowData);
                }
            }
        }
    }
    return lastUpdateStatus;
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
        TflStation* data = dataSource.getStationData();
        if (data->numServices > 0) {
            activeLayout->render(display);
        } else {
            activeLayout->headWidget.render(display);
            display.setFont(Underground10);
            display.drawStr(10, 35, "No arrivals scheduled.");
            activeLayout->msgWidget.render(display);
        }
    }
}

void TfLBoard::renderAnimationUpdate(U8G2& display, uint32_t currentMillis) {
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
