/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/boards/tflBoard/tflBoard.cpp
 * Description: Implementation of TfL Tube board logic.
 */

#include <appContext.hpp>
#include "tflBoard.hpp"
#include <logger.hpp>

extern const char tflAttribution[];
extern const uint8_t Underground10[];

/**
 * @brief Constructs the TfL Board and its default layout.
 * @param contextPtr Pointer to the global application context.
 */
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

/**
 * @brief Cleanup layout allocations.
 */
TfLBoard::~TfLBoard() {
    if (activeLayout) delete activeLayout;
}

/**
 * @brief Lifecycle hook for activation. Configures the data source 
 *        and prepares the layout widgets.
 */
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

/**
 * @brief Lifecycle hook for deactivation.
 */
void TfLBoard::onDeactivate() {}

/**
 * @brief Apply board-specific settings from the global configuration.
 * @param config The BoardConfig struct.
 */
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

/**
 * @brief Main logic tick for the board and its active layout.
 * @param ms Current system time in milliseconds.
 */
void TfLBoard::tick(uint32_t ms) {
    if (activeLayout) activeLayout->tick(ms);
}

/**
 * @brief Triggers or polls the background data fetch status.
 * @return Status code (0 = Success, 9 = Pending).
 */
int TfLBoard::updateData() {
    // --- Step 1: Handle Async Latency ---
    if (lastUpdateStatus == UPD_PENDING) {
        lastUpdateStatus = dataSource.getLastUpdateStatus();
        if (lastUpdateStatus == UPD_PENDING) {
            return UPD_PENDING;
        }
    } else {
        // --- Step 2: Validate Config ---
        if (!config.complete) {
            LOG_WARN("DISPLAY", "TfL Board: Skipping updateData() - Configuration incomplete.");
            return 7;
        }
        
        // --- Step 3: Initiation ---
        LOG_INFO("DISPLAY", "TfL Board: Starting data update...");
        lastUpdateStatus = dataSource.updateData();
        if (lastUpdateStatus == UPD_PENDING) {
            return UPD_PENDING;
        }
    }

    // --- Step 4: Error Tracking ---
    if (lastUpdateStatus != 0 && lastUpdateStatus != 1) {
        LOG_WARN("DISPLAY", "TfL Board: Data update failed with status: " + String(lastUpdateStatus));
        if (lastUpdateStatus > 2) {
            consecutiveErrors++;
        }
    } else {
        LOG_INFO("DISPLAY", "TfL Board: Data update finished successfully.");
        consecutiveErrors = 0;
    }

    // --- Step 5: Data Push to UI ---
    if (lastUpdateStatus == 0) { // UPD_SUCCESS
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

/**
 * @brief Renders the full board including error state handling.
 * @param display Reference to U8g2.
 */
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
            activeLayout->servicesWidget.setVisible(true);
            activeLayout->noDataLabel.setVisible(false);
        } else {
            activeLayout->servicesWidget.setVisible(false);
            activeLayout->noDataLabel.setVisible(true);
        }
        activeLayout->render(display);
    }
}

/**
 * @brief Targeted high-speed animation updates for scrollers and clocks.
 * @param display Reference to U8g2.
 * @param currentMillis Current system time in milliseconds.
 */
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
