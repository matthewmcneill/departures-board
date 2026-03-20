/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * Module: modules/displayManager/boards/nationalRailBoard/iNationalRailLayout.cpp
 * Description: Implementation of iNationalRailLayout constructor.
 */

#include "iNationalRailLayout.hpp"
#include <appContext.hpp>

iNationalRailLayout::iNationalRailLayout(appContext* context) 
    : iBoardLayout(context),
      headWidget(0, 0, 256, 12),
      row0Time(0, 12, 60, 14),
      row0Dest(60, 12, 196, 14),
      servicesWidget(0, 26, 256, 39),
      msgWidget(0, 52, 256, 12),
      sysClock(&context->getTimeManager(), 200, 0, 56, 14) {}
