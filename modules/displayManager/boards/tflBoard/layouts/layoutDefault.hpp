/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * Module: modules/displayManager/boards/tflBoard/layouts/layoutDefault.hpp
 * Description: Default layout for TfL Tube, migrating hardcoded board logic.
 */

#ifndef TFL_LAYOUT_DEFAULT_HPP
#define TFL_LAYOUT_DEFAULT_HPP

#include "../iTflLayout.hpp"

/**
 * @brief Default TfL Tube layout.
 */
class layoutTflDefault : public iTflLayout {
public:
    layoutTflDefault(appContext* context);
};

#endif // TFL_LAYOUT_DEFAULT_HPP
