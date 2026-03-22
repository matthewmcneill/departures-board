/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/boards/tflBoard/layouts/layoutDefault.hpp
 * Description: Default layout for TfL Underground, providing a pixel-perfect 
 *              recreation of the legacy v2.5 board.
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
