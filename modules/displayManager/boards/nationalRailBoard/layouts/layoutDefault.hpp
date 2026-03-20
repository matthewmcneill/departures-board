/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * Module: modules/displayManager/boards/nationalRailBoard/views/viewDefault.hpp
 * Description: Default layout for National Rail, migrating hardcoded board logic.
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
