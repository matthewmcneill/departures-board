/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * Module: modules/displayManager/boards/busBoard/iBusLayout.cpp
 * Description: Implementation of iBusLayout constructor logic.
 */

#include "iBusLayout.hpp"
#include <appContext.hpp>

iBusLayout::iBusLayout(appContext* context)
    : iBoardLayout(context),
      locationAndFilters(0, 0, 0, 0),
      weather(context, 0, 0, 0, 0),
      wifiWarning(0, 0),
      sysClock(&context->getTimeManager(), 0, 0, 0, 0),
      row0Widget(0, 0, 0, 0),
      servicesWidget(0, 0, 0, 0),
      msgWidget(0, 0, 0, 0),
      noDataLabel(0, 0, 0, 0) {
    noDataLabel.setVisible(false);
}
