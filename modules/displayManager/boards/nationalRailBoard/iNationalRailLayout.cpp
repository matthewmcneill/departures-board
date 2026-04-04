/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * Module: modules/displayManager/boards/nationalRailBoard/iNationalRailLayout.cpp
 * Description: Implementation of iNationalRailLayout constructor.
 *
 * Exported Functions/Classes:
 * - iNationalRailLayout: [Class implementation]
 *   - iNationalRailLayout(): Constructor for initializing layout sub-widgets.
 */

#include "iNationalRailLayout.hpp"
#include <appContext.hpp>
#define DIMMED_BRIGHTNESS 15

/**
 * @brief Construct a new iNationalRailLayout.
 * Initializes all sub-widgets to zero-bounds; these are later repositioned by the specific JSON layout binding.
 * @param context Pointer to the central appContext.
 */
iNationalRailLayout::iNationalRailLayout(appContext* context)
    : iBoardLayout(context),
      locationAndFilters(0, 0, 0, 0),
      weather(context, 0, 0, 0, 0),
      wifiWarning(0, 0),
      sysClock(&context->getTimeManager(), 0, 0, 0, 0),
      row0Widget(0, 0, 0, 0),
      servicesWidget(0, 0, 0, 0),
      msgWidget(0, 0, 0, 0),
      noDataLabel(0, 0, 0, 0),
      platformWidget(0, 0, 0, 0) {
    noDataLabel.setVisible(false);
}
