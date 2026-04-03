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
 * Module: modules/displayManager/boards/tflBoard/tflBoard.cpp
 * Description: Implementation of TfL Tube board logic.
 *
 * Exported Functions/Classes:
 * - TfLBoard: [Class implementation]
 *   - onActivate() / onDeactivate(): Lifecycle hooks for display transitions.
 *   - tick() / render(): Logic and drawing entry points.
 *   - updateData(): Initiates JSON fetch from TfL API.
 *   - configure(): Applies BoardConfig settings.
 */

#include "tflBoard.hpp"
#include <appContext.hpp>
#include <logger.hpp>

const char tflAttribution[] = "Powered by TfL Open Data"; // Attribution for Transport for London
extern const uint8_t Underground10[];

/**
 * @brief Constructs the TfL Board and its default layout.
 * @param contextPtr Pointer to the global application context.
 */
TfLBoard::TfLBoard(appContext *contextPtr)
    : context(contextPtr), activeLayout(nullptr), lastUpdate(0), lastRenderedHash(0) {

  // Ensure activeLayout is explicitly null until configure() is called
  activeLayout = nullptr;

  tflAppkey[0] = '\0';
  tubeId[0] = '\0';
  tubeName[0] = '\0';

  // Register this board's source with the predictive DataManager
  if (context) {
    context->getDataManager().registerSource(&dataSource);
  }
}

/**
 * @brief Cleanup allocated resources and layouts.
 */
TfLBoard::~TfLBoard() {
  if (context) {
    context->getDataManager().unregisterSource(&dataSource);
  }
  if (activeLayout)
    delete activeLayout;
}

/**
 * @brief Called when the board becomes the active display.
 * Configures the data source and prepares the layout widgets.
 */
void TfLBoard::onActivate() {
  dataSource.configure(tubeId, tflAppkey);

  if (activeLayout) {
    activeLayout->locationAndFilters.setLocation(tubeName);
    activeLayout->locationAndFilters.setFilters(
        ""); // Currently no global filters for TfL

    // Configure message pools
    activeLayout->msgWidget.clearPools();
    if (context->getConfigManager().getConfig().prioritiseRss) {
      if (context)
        activeLayout->msgWidget.addMessagePool(
            &context->getGlobalMessagePool());
      activeLayout->msgWidget.addMessagePool(dataSource.getMessagesData());
    } else {
      activeLayout->msgWidget.addMessagePool(dataSource.getMessagesData());
      if (context)
        activeLayout->msgWidget.addMessagePool(
            &context->getGlobalMessagePool());
    }

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
void TfLBoard::configure(const BoardConfig &config) {
  this->config = config;

  // Destroy existing layout to prevent heap leaks on consecutive updates
  if (activeLayout) {
    delete activeLayout;
    activeLayout = nullptr;
  }

  // Evaluate requested layout against available subclasses
  // (No alternatives yet for TfL, fallback to default)
  activeLayout = new layoutTflDefault(context);

  if (context) {
    ApiKey *key = context->getConfigManager().getKeyById(config.apiKeyId);
    if (key)
      setTflAppkey(key->token);
    else
      tflAppkey[0] = '\0';
  }
  if (strcmp(config.id, "HUBTCR") == 0) {
    setTubeId("940GZZLUTCR");
  } else {
    setTubeId(config.id);
  }
  setTubeName(config.name);
  dataSource.setFilter(config.filter);
  dataSource.setDirectionFilter(config.tflDirectionFilter);

  // Inject the credentials down into the API caller so background sweeps work
  // successfully
  dataSource.configure(tubeId, tflAppkey);
}

/**
 * @brief Main logic tick for the board and its active layout.
 * @param ms Current system time in milliseconds.
 */
void TfLBoard::tick(uint32_t ms) {
  if (activeLayout)
    activeLayout->tick(ms);
}

/**
 * @brief Trigger an asynchronous data refresh from TfL APIs.
 * @return UpdateStatus code.
 */
UpdateStatus TfLBoard::updateData() {
  // --- Step 1: Handle Async Latency ---
  if (lastUpdateStatus == UpdateStatus::PENDING) {
    lastUpdateStatus = dataSource.getLastUpdateStatus();
    if (lastUpdateStatus == UpdateStatus::PENDING) {
      return UpdateStatus::PENDING;
    }
  } else {
    // --- Step 2: Validate Config ---
    if (!config.complete) {
      LOG_WARN("DISPLAY",
               "TfL Board: Skipping updateData() - Configuration incomplete.");
      return UpdateStatus::DATA_ERROR;
    }

    // --- Step 3: Initiation ---
    LOG_VERBOSE("DISPLAY", "TfL Board: Starting data update...");
    lastUpdateStatus = dataSource.updateData();
    if (lastUpdateStatus == UpdateStatus::PENDING) {
      return UpdateStatus::PENDING;
    }
  }

  // --- Step 4: Error Tracking ---
  if (lastUpdateStatus != UpdateStatus::SUCCESS && lastUpdateStatus != UpdateStatus::NO_CHANGE) {
    LOG_WARN("DISPLAY", "TfL Board: Data update failed with status: " +
                            String((int)lastUpdateStatus));
    if (lastUpdateStatus >= UpdateStatus::TIMEOUT) {
      consecutiveErrors++;
    }
  } else {
    LOG_VERBOSE("DISPLAY", "TfL Board: Data update finished successfully.");
    consecutiveErrors = 0;
  }

  // --- Step 5: Data Push to UI ---
  if (lastUpdateStatus == UpdateStatus::SUCCESS) { // UpdateStatus::SUCCESS
    dataSource.lockData();
    TflStation *data = dataSource.getStationData();
    if (activeLayout && data->contentHash != lastRenderedHash) {
      activeLayout->row0Widget.clearRows();
      activeLayout->servicesWidget.clearRows();
      if (data->numServices > 0) {
        for (int i = 0; i < data->numServices; i++) {
          // --- Step 3: Populate 4-Column Layout ---
          // Column Order: 0: OrderNum, 1: Line, 2: Destination, 3: Time
          // The orderNum is a stable pointer assigned by the DataSource.
          const char *rowData[4] = {
              config.showServiceOrdinals ? data->service[i].orderNum : "",
              data->service[i].lineName, data->service[i].destination,
              data->service[i].expectedTime};
          activeLayout->row0Widget.addRow(rowData);
          activeLayout->servicesWidget.addRow(rowData);
        }
      }
      lastRenderedHash = data->contentHash;
    }
    dataSource.unlockData();
  }
  return lastUpdateStatus;
}

/**
 * @brief Main rendering hook for the entire board.
 * @param display Reference to the global U8g2 instance.
 */
void TfLBoard::render(U8G2 &display) {
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
    dataSource.lockData();
    TflStation *data = dataSource.getStationData();
    if (data->numServices > 0) {
      activeLayout->row0Widget.setVisible(true);
      activeLayout->servicesWidget.setVisible(true);
      activeLayout->noDataLabel.setVisible(false);
    } else {
      if (lastUpdateStatus == UpdateStatus::NO_DATA || lastUpdateStatus == UpdateStatus::PENDING) {
        activeLayout->noDataLabel.setText("Loading data...");
      } else {
        activeLayout->noDataLabel.setText("No arrivals scheduled.");
      }
      activeLayout->row0Widget.setVisible(false);
      activeLayout->servicesWidget.setVisible(false);
      activeLayout->noDataLabel.setVisible(true);
    }
    activeLayout->render(display);
    dataSource.unlockData();
  }
}

/**
 * @brief Targeted high-speed animation updates for scrollers and clocks.
 * @param display Reference to the global U8g2 instance.
 * @param currentMillis Current system time in milliseconds.
 */
void TfLBoard::renderAnimationUpdate(U8G2 &display, uint32_t currentMillis) {
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

bool TfLBoard::isScrollFinished() {
  if (activeLayout)
    return activeLayout->msgWidget.isScrollFinished();
  return true;
}
