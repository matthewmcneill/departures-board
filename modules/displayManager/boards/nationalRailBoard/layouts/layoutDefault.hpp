/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/boards/nationalRailBoard/layouts/layoutDefault.hpp
 * Description: Default layout for National Rail, providing a pixel-perfect 
 *              recreation of the legacy v2.5 board while utilizing the 
 *              v3.0 widget-based architecture.
 */

#pragma once
#ifndef NR_LAYOUT_DEFAULT_HPP
#define NR_LAYOUT_DEFAULT_HPP

#include "../iNationalRailLayout.hpp"

/**
 * @brief Default National Rail layout.
 *        Pixel-perfect migration of the v2.5 hardcoded board logic.
 */
class layoutNrDefault : public iNationalRailLayout {
private:
    char cachedOrdinals[16][5];
    bool viaToggle = false;
    uint32_t nextViaToggle = 0;

public:
    /**
     * @brief Constructor for the default view.
     * @param context Application context for DI.
     */
    layoutNrDefault(appContext* context);

    virtual void tick(uint32_t currentMillis) override;
    virtual void render(U8G2& display) override;
};

#endif // NR_LAYOUT_DEFAULT_HPP
