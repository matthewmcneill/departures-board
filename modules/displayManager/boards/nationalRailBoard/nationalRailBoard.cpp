/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons
 * Attribution-NonCommercial-ShareAlike 4.0 International. To view a copy of
 * this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/boards/nationalRailBoard/nationalRailBoard.cpp
 * Description: Implementation of National Rail controller logic and polymorphic data binding.
 *
 * Exported Functions/Classes:
 * - NationalRailBoard: Concrete implementation of a departures board for National Rail data.
 *   - onActivate() / onDeactivate(): Lifecycle hooks for display transitions.
 *   - tick() / render(): Logic and drawing entry points.
 *   - updateData(): Triggers asynchronous fetch from the polymorphic activeDataProvider.
 *   - configure(): Loads BoardConfig structure and configures RDM/DARWIN dynamic instances.
 */

#include "nationalRailBoard.hpp"
#include "nrDARWINDataProvider.hpp"
#include "nrRDMDataProvider.hpp"
#include <appContext.hpp>
#include <logger.hpp>
#include "layouts/layoutSWR.hpp"
#include "layouts/layoutGadec.hpp"

extern const uint8_t NatRailTall12[];
extern const uint8_t NatRailSmall9[];

/**
 * @brief Constructs the National Rail Board and its default layout.
 * @param contextPtr Pointer to the global application context.
 */
NationalRailBoard::NationalRailBoard(appContext *contextPtr)
    : context(contextPtr), activeDataSource(nullptr), activeLayout(nullptr), lastUpdate(0),
      viaToggle(false), nextViaToggle(0), lastRenderedHash(0) {

  // Ensure activeLayout is explicitly null until configure() is called
  activeLayout = nullptr;

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
  if (context && activeDataSource) {
    context->getDataManager().unregisterSource(activeDataSource);
  }
  if (activeDataSource) {
    delete activeDataSource;
  }
  if (activeLayout)
    delete activeLayout;
}

/**
 * @brief Lifecycle hook for activation. Initializes the SOAP client
 *        and binds widgets to the data source alerts.
 */
void NationalRailBoard::onActivate() {
  LOG_INFO("DISPLAY", "NR Board: onActivate() called.");

  // --- Step 1: Data Source Initialization ---
  // (WSDL Initialization is delegated to DataManager on safe background thread)

  // --- Step 2: Configuration Injection ---
  activeDataSource->configure(nrToken, crsCode, platformFilter, callingCrsCode,
                       nrTimeOffset);

  // --- Step 3: Widget Binding ---
  if (context && activeLayout) {
    // Register message pools (global plus source-specific alerts)
    activeLayout->msgWidget.clearPools();
    if (context->getConfigManager().getConfig().prioritiseRss) {
      activeLayout->msgWidget.addMessagePool(&context->getGlobalMessagePool());
      activeLayout->msgWidget.addMessagePool(activeDataSource->getMessagesData());
    } else {
      activeLayout->msgWidget.addMessagePool(activeDataSource->getMessagesData());
      activeLayout->msgWidget.addMessagePool(&context->getGlobalMessagePool());
    }

    // Initial attribution text
    const char* attr = activeDataSource->getAttributionString();
    if (attr) {
        activeLayout->msgWidget.setInterleavedMessage(attr);
    }
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
void NationalRailBoard::configure(const BoardConfig &config) {
  this->config = config;

  // Destroy existing layout to prevent heap leaks on consecutive updates
  if (activeLayout) {
    delete activeLayout;
    activeLayout = nullptr;
  }

  // Evaluate requested layout against available subclasses
  if (strcmp(config.layout, "replica") == 0) {
    activeLayout = new layoutNrReplica(context);
  } else if (strcmp(config.layout, "swr") == 0) {
    activeLayout = new layoutNrSWR(context);
  } else if (strcmp(config.layout, "gadec") == 0) {
    activeLayout = new layoutNrGadec(context);
  } else {
    // Fallback or "default"
    activeLayout = new layoutNrDefault(context);
  }

  ApiKey *key = nullptr;
  if (context) {
    key = context->getConfigManager().getKeyById(config.apiKeyId);
    if (key)
      setNrToken(key->token);
    else
      nrToken[0] = '\0';
  }
  setCrsCode(config.id);
  setPlatformFilter(config.filter);
  setCallingCrsCode(config.secondaryId);
  setCallingStation(config.secondaryName);
  setNrTimeOffset(config.timeOffset);
  setStationLat(config.lat);
  setStationLon(config.lon);

  // Unregister existing data source
  if (context && activeDataSource) {
      context->getDataManager().unregisterSource(activeDataSource);
  }
  if (activeDataSource) {
      delete activeDataSource;
      activeDataSource = nullptr;
  }

  // Instantiate proper provider based on Key Type
  if (key && strcmp(key->type, "rdm") == 0) {
      activeDataSource = new nrRDMDataProvider();
  } else {
      nrDARWINDataProvider* darwin = new nrDARWINDataProvider();
      // Initialize Darwin SOAP WSDL Endpoints
      darwin->init("lite.realtime.nationalrail.co.uk", "/OpenLDBWS/wsdl.aspx?ver=2021-11-01");
      activeDataSource = darwin;
  }

  if (context) {
      context->getDataManager().registerSource(activeDataSource);
  }

  activeDataSource->configure(nrToken, crsCode, platformFilter, callingCrsCode, nrTimeOffset);
}

/**
 * @brief Main logic loop. Handles timing for "Via" point toggles and layout animations.
 * @param ms Current system time in milliseconds.
 */
void NationalRailBoard::tick(uint32_t ms) {
  if (ms > nextViaToggle) {
    viaToggle = !viaToggle;
    nextViaToggle = ms + 4000;
    activeDataSource->lockData();
    populateServices(true); // Repopulate only row 0 to flip via points
    activeDataSource->unlockData();
  }

  if (activeLayout)
    activeLayout->tick(ms);
}

/**
 * @brief Trigger an asynchronous data refresh from National Rail LDBWS.
 * Calls into the DataManager to queue a priority SOAP request.
 * @return UpdateStatus code.
 */
UpdateStatus NationalRailBoard::updateData() {
  // --- Step 1: Pending Result Status ---
  if (lastUpdateStatus == UpdateStatus::PENDING) {
    lastUpdateStatus = activeDataSource->getLastUpdateStatus();
    if (lastUpdateStatus == UpdateStatus::PENDING) {
      return UpdateStatus::PENDING;
    }
  } else {
    // --- Step 2: Pre-Fetch Validation ---
    if (!config.complete) {
      LOG_WARN("DISPLAY",
               "NR Board: Skipping updateData() - Configuration incomplete.");
      return UpdateStatus::DATA_ERROR; // Unconfigured
    }

    // (WSDL Initialization is handled safely inside the Data Source)

    // --- Step 3: Initiation ---
    LOG_VERBOSE("DISPLAY", "NR Board: Starting data update...");
    lastUpdateStatus = activeDataSource->updateData();
    if (lastUpdateStatus == UpdateStatus::PENDING) {
      return UpdateStatus::PENDING;
    }
  }

  // --- Step 4: Post-Fetch Error Tracking ---
  if (lastUpdateStatus != UpdateStatus::SUCCESS && lastUpdateStatus != UpdateStatus::NO_CHANGE) {
    LOG_WARN("DISPLAY", "NR Board: Data update failed with status: " +
                            String((int)lastUpdateStatus));
    if (lastUpdateStatus >= UpdateStatus::TIMEOUT) {
      consecutiveErrors++;
    }
  } else {
    LOG_VERBOSE("DISPLAY", "NR Board: Data update finished successfully.");
    consecutiveErrors = 0;
  }

  if (lastUpdateStatus == UpdateStatus::SUCCESS) { // UpdateStatus::SUCCESS
    activeDataSource->lockData();
    // Update header once we have the station name
    NationalRailStation *data = activeDataSource->getStationData();
    if (activeLayout) {
      activeLayout->locationAndFilters.setLocation(data->location);

      String filterText = "";
      if (callingStation[0] != '\0') {
        filterText += "to " + String(callingStation);
      }
      if (platformFilter[0] != '\0') {
        if (filterText.length() > 0)
          filterText += " ";
        filterText += "[Plat " + String(platformFilter) + "]";
        activeLayout->platformWidget.setText(platformFilter);
      } else {
        activeLayout->platformWidget.setText("A");
      }
      activeLayout->locationAndFilters.setFilters(filterText.c_str());
    }

    if (activeLayout && data->contentHash != lastRenderedHash) {
      // Trigger scrolling of calling points if they changed
      if (data->numServices > 0) {
        String msg = data->firstServiceCalling;
        if (config.showLastSeenLocation && data->firstServiceLastSeen[0]) {
          if (msg.length() > 0)
            msg += ". ";
          msg += "Last seen at ";
          msg += data->firstServiceLastSeen;
          msg += ".";
        }
        if (msg.length() > 0) {
          activeLayout->msgWidget.setInterleavedMessage(msg.c_str());
        } else {
          const char* attr = activeDataSource->getAttributionString();
          if (attr) {
            activeLayout->msgWidget.setInterleavedMessage(attr);
          }
        }
      }

      // Populate services lists (row 0 and subsequent)
      populateServices(false);
      lastRenderedHash = data->contentHash;
    }
    activeDataSource->unlockData();
  }
  return lastUpdateStatus;
}

/**
 * @brief Main rendering entry point for the National Rail board.
 * Renders errors, setup help, or the active layout based on state.
 * @param display Reference to the global U8g2 graphics instance.
 */
void NationalRailBoard::render(U8G2 &display) {
  if (context && context->getWifiManager().isWifiPersistentError()) {
    iDisplayBoard *wifiError = context->getDisplayManager().getSystemBoard(
        SystemBoardId::SYS_ERROR_WIFI);
    if (wifiError) {
      wifiError->render(display);
      return;
    }
  }

  if (!config.complete) {
    if (context) {
      iDisplayBoard *help = context->getDisplayManager().getSystemBoard(
          SystemBoardId::SYS_SETUP_HELP);
      if (help)
        help->render(display);
    }
    return;
  }

  if (lastUpdateStatus >= UpdateStatus::TIMEOUT && consecutiveErrors >= 3) {
    if (context) {
      SystemBoardId id =
          context->getDisplayManager().mapErrorToId(lastUpdateStatus);
      iDisplayBoard *errBoard = context->getDisplayManager().getSystemBoard(id);
      if (errBoard) {
        errBoard->render(display);
        return;
      }
    }
  }

  if (activeLayout) {
    activeDataSource->lockData();
    NationalRailStation *data = activeDataSource->getStationData();
    if (data->numServices > 0) {
      activeLayout->row0Widget.setVisible(true);
      activeLayout->servicesWidget.setVisible(true);
      activeLayout->noDataLabel.setVisible(false);
    } else {
      if (lastUpdateStatus == UpdateStatus::NO_DATA || lastUpdateStatus == UpdateStatus::PENDING) {
        activeLayout->noDataLabel.setText("Loading data...");
      } else {
        activeLayout->noDataLabel.setText("No services found.");
      }
      activeLayout->row0Widget.setVisible(false);
      activeLayout->servicesWidget.setVisible(false);
      activeLayout->noDataLabel.setVisible(true);
    }
    activeLayout->render(display);
    activeDataSource->unlockData();
  }
}

/**
 * @brief Populate generic service widgets from the central data model.
 */
void NationalRailBoard::populateServices(bool row0Only) {
  if (!activeLayout)
    return;

  NationalRailStation *data = activeDataSource->getStationData();
  activeLayout->row0Widget.clearRows();
  if (!row0Only) {
      activeLayout->servicesWidget.clearRows();
  }

  if (data->numServices > 0) {
    for (int i = 0; i < data->numServices; i++) {
      if (i >= 17)
        break; // Array limit

      char *ordinalRef;
      if (i > 0) {
        sprintf(cachedOrdinals[i - 1], "%d%s", i + 1,
                (i == 1 ? "nd" : (i == 2 ? "rd" : "th")));
        ordinalRef = cachedOrdinals[i - 1];
      } else {
        strcpy(firstOrdinal, "1st");
        ordinalRef = firstOrdinal;
      }

      const char *dest = (i == 0 && viaToggle && data->service[0].via[0])
                             ? data->service[0].via
                             : data->service[i].destination;

      const char *rowData[5] = {ordinalRef, data->service[i].sTime, dest,
                                data->service[i].platform,
                                data->service[i].etd};

      if (i == 0 || !row0Only) {
          activeLayout->row0Widget.addRow(rowData);
      }
      if (!row0Only) {
          activeLayout->servicesWidget.addRow(rowData);
      }
    }
  }
}

/**
 * @brief Targeted high-speed animation updates for scrollers and clocks.
 * @param display Reference to U8g2.
 * @param currentMillis Current system time in milliseconds.
 */
void NationalRailBoard::renderAnimationUpdate(U8G2 &display,
                                              uint32_t currentMillis) {
  if (context && context->getWifiManager().isWifiPersistentError())
    return;
  if (!config.complete)
    return;
  if (lastUpdateStatus >= UpdateStatus::TIMEOUT && consecutiveErrors >= 3) {
    if (context) {
      SystemBoardId id =
          context->getDisplayManager().mapErrorToId(lastUpdateStatus);
      iDisplayBoard *errBoard = context->getDisplayManager().getSystemBoard(id);
      if (errBoard) {
        errBoard->renderAnimationUpdate(display, currentMillis);
        return;
      }
    }
  }

  if (activeLayout)
    activeLayout->renderAnimationUpdate(display, currentMillis);
}

bool NationalRailBoard::isScrollFinished() {
  if (activeLayout)
    return activeLayout->msgWidget.isScrollFinished();
  return true;
}
