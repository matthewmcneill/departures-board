/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/boards/nationalRailBoard/nationalRailBoard.cpp
 * Description: Implementation of National Rail controller logic and widget binding.
 *
 * Exported Functions/Classes:
 * - NationalRailBoard: Controller for NR departure boards.
 *   - onActivate(): Lifecycle hook for activation.
 *   - onDeactivate(): Lifecycle hook for deactivation.
 *   - configure(const BoardConfig& config): Apply settings.
 *   - tick(uint32_t ms): Periodic logic update.
 *   - updateData(): Fetch background data.
 *   - render(U8G2& display): Standard render pass.
 *   - renderAnimationUpdate(U8G2& display, uint32_t currentMillis): Animation pass.
 */

#include <appContext.hpp>
#include "nationalRailBoard.hpp"
#include <logger.hpp>

extern const char nrAttributionn[];
extern const uint8_t NatRailTall12[];
extern const uint8_t NatRailSmall9[];

/**
 * @brief Constructs the National Rail Board and its default layout.
 * @param contextPtr Pointer to the global application context.
 */
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

/**
 * @brief Cleanup layout allocations.
 */
NationalRailBoard::~NationalRailBoard() {
    if (activeLayout) delete activeLayout;
}

/**
 * @brief Lifecycle hook for activation. Initializes the SOAP client 
 *        and binds widgets to the data source alerts.
 */
void NationalRailBoard::onActivate() {
    LOG_INFO("DISPLAY", "NR Board: onActivate() called.");
    
    // --- Step 1: Data Source Initialization ---
    // Defer initialization if WiFi is not connected to avoid blocking the main thread during boot.
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
    
    // --- Step 2: Configuration Injection ---
    dataSource.configure(nrToken, crsCode, platformFilter, callingCrsCode, nrTimeOffset);
    
    // --- Step 3: Widget Binding ---
    if (context && activeLayout) {
        // Register message pools (global plus source-specific alerts)
        activeLayout->msgWidget.addMessagePool(&context->getGlobalMessagePool());
        activeLayout->msgWidget.addMessagePool(dataSource.getMessagesData());
        
        // Initial attribution text
        activeLayout->msgWidget.setText(nrAttributionn);
    }
    lastUpdate = 0;
}

/**
 * @brief Lifecycle hook for deactivation.
 */
void NationalRailBoard::onDeactivate() {}

/**
 * @brief Apply board-specific settings (CRS, tokens) from configuration.
 * @param config The BoardConfig struct.
 */
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
    
    // Inject the credentials and instantly init the static endpoints so background sweeps work successfully
    dataSource.init("lite.realtime.nationalrail.co.uk", "/OpenLDBWS/wsdl.aspx?ver=2021-11-01", raildataYieldWrapper);
    dataSource.configure(nrToken, crsCode, platformFilter, callingCrsCode, nrTimeOffset);
}

/**
 * @brief Logic update for the board, including destination/via toggling.
 * @param ms Current system time in milliseconds.
 */
void NationalRailBoard::tick(uint32_t ms) {
    if (ms > nextViaToggle) {
        viaToggle = !viaToggle;
        nextViaToggle = ms + 4000;
        populateServices(); // Repopulate to flip via points
    }

    if (activeLayout) activeLayout->tick(ms);
}

/**
 * @brief Triggers or polls the background SOAP data fetch status.
 * @return Status code (0 = Success, 9 = Pending).
 */
int NationalRailBoard::updateData() {
    // --- Step 1: Pending Result Status ---
    if (lastUpdateStatus == UPD_PENDING) {
        lastUpdateStatus = dataSource.getLastUpdateStatus();
        if (lastUpdateStatus == UPD_PENDING) {
            return UPD_PENDING;
        }
    } else {
        // --- Step 2: Pre-Fetch Validation ---
        if (!config.complete) {
            LOG_WARN("DISPLAY", "NR Board: Skipping updateData() - Configuration incomplete.");
            return 7; // Unconfigured
        }

        // Deferred init if WiFi joined late
        if (WiFi.status() == WL_CONNECTED && !dataSource.isInitialized()) {
            LOG_INFO("DISPLAY", "NR Board: Performing deferred dataSource.init()...");
            dataSource.init("lite.realtime.nationalrail.co.uk", "/OpenLDBWS/wsdl.aspx?ver=2021-11-01", nullptr);
        }

        // --- Step 3: Initiation ---
        LOG_INFO("DISPLAY", "NR Board: Starting data update...");
        lastUpdateStatus = dataSource.updateData();
        if (lastUpdateStatus == UPD_PENDING) {
            return UPD_PENDING;
        }
    }

    // --- Step 4: Post-Fetch Error Tracking ---
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
        if (activeLayout) {
            activeLayout->stationName.setText(data->location);
            
            String filterText = "";
            if (callingStation[0] != '\0') {
                filterText += "to " + String(callingStation);
            }
            if (platformFilter[0] != '\0') {
                if (filterText.length() > 0) filterText += " ";
                filterText += "[Plat " + String(platformFilter) + "]";
            }
            activeLayout->filterInfo.setText(filterText.c_str());
        }

        // Trigger scrolling of calling points if they changed
        if (data->numServices > 0 && activeLayout && data->service[0].calling[0]) {
            activeLayout->msgWidget.setText(data->service[0].calling);
        }
        
        // Populate services lists (row 0 and subsequent)
        populateServices();
    }
    return lastUpdateStatus;
}

/**
 * @brief Renders the full National Rail board including Row 0 and secondary list.
 * @param display Reference to U8g2.
 */
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
            activeLayout->row0Widget.setVisible(true);
            activeLayout->servicesWidget.setVisible(true);
            activeLayout->noDataLabel.setVisible(false);
        } else {
            if (lastUpdateStatus == -1 || lastUpdateStatus == 9) {
                activeLayout->noDataLabel.setText("Loading data...");
            } else {
                activeLayout->noDataLabel.setText("No services found.");
            }
            activeLayout->row0Widget.setVisible(false);
            activeLayout->servicesWidget.setVisible(false);
            activeLayout->noDataLabel.setVisible(true);
        }
        activeLayout->render(display);
    }
}

/**
 * @brief Populate generic service widgets from the central data model.
 */
void NationalRailBoard::populateServices() {
    if (!activeLayout) return;
    
    NationalRailStation* data = dataSource.getStationData();
    activeLayout->row0Widget.clearRows();
    activeLayout->servicesWidget.clearRows();
    
    if (data->numServices > 0) {
        for (int i = 0; i < data->numServices; i++) {
            if (i >= 17) break; // Array limit
            
            char* ordinalRef;
            if (i > 0) {
                sprintf(cachedOrdinals[i-1], "%d%s", i+1, (i==1?"nd":(i==2?"rd":"th")));
                ordinalRef = cachedOrdinals[i-1];
            } else {
                strcpy(firstOrdinal, "1st");
                ordinalRef = firstOrdinal;
            }
            
            const char* dest = (i == 0 && viaToggle && data->service[0].via[0]) ? data->service[0].via : data->service[i].destination;
            
            const char* rowData[5] = {
                ordinalRef,
                data->service[i].sTime,
                dest,
                data->service[i].platform,
                data->service[i].etd
            };
            
            activeLayout->row0Widget.addRow(rowData);
            activeLayout->servicesWidget.addRow(rowData);
        }
    }
}

/**
 * @brief Targeted high-speed animation updates for scrollers and clocks.
 * @param display Reference to U8g2.
 * @param currentMillis Current system time in milliseconds.
 */
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
