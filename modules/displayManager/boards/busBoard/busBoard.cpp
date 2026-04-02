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
 * Module: modules/displayManager/boards/busBoard/busBoard.cpp
 * Description: Implementation of Bus arrival controller logic.
 *
 * Exported Functions/Classes:
 * - BusBoard: [Class implementation]
 *   - BusBoard(): Constructor, registers with DataManager.
 *   - ~BusBoard(): Destructor, unregisters source.
 *   - onActivate() / onDeactivate(): Lifecycle hooks for display transitions.
 *   - configure(): Sets up layouts and API credentials.
 *   - tick() / render(): Standard entry points for logic and drawing.
 *   - updateData(): High-level orchestration of the TfL API fetch.
 */

#include "busBoard.hpp"
#include <appContext.hpp>
#include <logger.hpp>

const char btAttribution[] =
    "Powered by bustimes.org";        ///< Board attribution text
extern const uint8_t Underground10[]; ///< Built-in font
extern const uint8_t NatRailSmall9[]; ///< Small font for scrolling messages

/**
 * @brief Constructs the BusBoard and configures widget positions.
 */
BusBoard::BusBoard(appContext *contextPtr)
    : context(contextPtr), activeLayout(nullptr), lastUpdate(0),
      needsRefresh(true) {

  // Ensure activeLayout is explicitly null until configure() is called
  activeLayout = nullptr;

  busAtco[0] = '\0';
  busName[0] = '\0';
  busFilter[0] = '\0';
  enableBus = false;

  // Register this board's source with the predictive DataManager
  if (context) {
    context->getDataManager().registerSource(&dataSource);
  }
}

/**
 * @brief Cleanup allocated resources and layouts.
 */
BusBoard::~BusBoard() {
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
void BusBoard::onActivate() {
  dataSource.configure(busAtco, busFilter);

  if (activeLayout) {
    activeLayout->locationAndFilters.setLocation(busName);
    activeLayout->locationAndFilters.setFilters(
        ""); // Currently no global filters for Bus

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
    activeLayout->msgWidget.setText(btAttribution);
  }
  lastUpdate = 0; // Trigger immediate update
}

/**
 * @brief Called when the display board is deactivated.
 */
void BusBoard::onDeactivate() {
  // Cleanup if needed
}

/**
 * @brief Apply board-specific settings (ATCO code, filter) from configuration.
 * @param config The BoardConfig struct.
 */
void BusBoard::configure(const BoardConfig &config) {
  this->config = config;

  // Destroy existing layout to prevent heap leaks on consecutive updates
  if (activeLayout) {
    delete activeLayout;
    activeLayout = nullptr;
  }

  // Evaluate requested layout against available subclasses
  // (No alternatives yet for Bus, fallback to default)
  activeLayout = new layoutBusDefault(context);

  setBusAtco(config.id);
  setBusName(config.name);
  setBusLat(config.lat);
  setBusLon(config.lon);
  setBusFilter(config.filter);

  // Inject the credentials down into the API caller so background sweeps work
  // successfully
  dataSource.configure(busAtco, busFilter);
}

/**
 * @brief Main logic tick for the board and its active layout.
 * @param ms Current system time in milliseconds.
 */
void BusBoard::tick(uint32_t ms) {
  if (activeLayout)
    activeLayout->tick(ms);
}

/**
 * @brief High-level update trigger for bus departure data.
 * Orchestrates the transition from PENDING to SUCCESS statuses
 * and populates the services widget with the fresh data set.
 * @return UpdateStatus code representing the result of the fetch.
 */
UpdateStatus BusBoard::updateData() {
  if (lastUpdateStatus == UpdateStatus::PENDING) {
    lastUpdateStatus = dataSource.getLastUpdateStatus();
    if (lastUpdateStatus == UpdateStatus::PENDING) {
      return UpdateStatus::PENDING;
    }
  } else {
    if (!config.complete) {
      LOG_WARN("DISPLAY",
               "Bus Board: Skipping updateData() - Configuration incomplete.");
      return UpdateStatus::DATA_ERROR;
    }

    LOG_INFO("DISPLAY", "Bus Board: Starting data update...");
    lastUpdateStatus = dataSource.updateData();
    if (lastUpdateStatus == UpdateStatus::PENDING) {
      return UpdateStatus::PENDING;
    }
  }

  needsRefresh = true;

  if (lastUpdateStatus != UpdateStatus::SUCCESS && lastUpdateStatus != UpdateStatus::NO_CHANGE) {
    LOG_WARN("DISPLAY", "Bus Board: Data update failed with status: " +
                            String((int)lastUpdateStatus));
    if (lastUpdateStatus >= UpdateStatus::TIMEOUT) {
      consecutiveErrors++;
    }
  } else {
    LOG_INFO("DISPLAY", "Bus Board: Data update finished successfully.");
    consecutiveErrors = 0;
  }

  if (lastUpdateStatus == UpdateStatus::SUCCESS) { // UpdateStatus::SUCCESS
    dataSource.lockData();
    BusStop *data = dataSource.getStationData();
    if (activeLayout) {
      activeLayout->servicesWidget.clearRows();
      if (data->numServices > 0) {
        for (int i = 0; i < data->numServices; i++) {
          // --- Step 3: Populate 4-Column Layout ---
          // Column Order: 0: OrderNum, 1: Route, 2: Destination, 3: Time
          // The orderNum is a stable pointer assigned by the DataSource.
          const char *rowData[4] = {
              config.showServiceOrdinals ? data->service[i].orderNum : "",
              data->service[i].routeNumber, data->service[i].destination,
              data->service[i].expectedTime[0] != '\0'
                  ? data->service[i].expectedTime
                  : data->service[i].sTime};
          activeLayout->servicesWidget.addRow(rowData);
        }
      }
    }
    dataSource.unlockData();
  }
  return lastUpdateStatus;
}

/**
 * @brief Main rendering hook for the entire board.
 * @param display Reference to the global U8g2 instance.
 */
void BusBoard::render(U8G2 &display) {
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
    BusStop *data = dataSource.getStationData();
    if (data->numServices > 0) {
      activeLayout->servicesWidget.setVisible(true);
      activeLayout->noDataLabel.setVisible(false);
    } else {
      if (lastUpdateStatus == UpdateStatus::NO_DATA || lastUpdateStatus == UpdateStatus::PENDING) {
        activeLayout->noDataLabel.setText("Loading data...");
      } else {
        activeLayout->noDataLabel.setText("No scheduled services.");
      }
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
void BusBoard::renderAnimationUpdate(U8G2 &display, uint32_t currentMillis) {
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

bool BusBoard::isScrollFinished() {
  if (activeLayout)
    return activeLayout->msgWidget.isScrollFinished();
  return true;
}
