/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/configManager/iConfigurable.hpp
 * Description: Interface for components that subscribe to configuration updates.
 *
 * Exported Functions/Classes:
 * - iConfigurable: Interface defining the reapplyConfig callback.
 * - iConfigurable::reapplyConfig: Callback triggered on configuration changes.
 */

#pragma once

#include "configManager.hpp"

/**
 * @brief Interface for any system component that needs to be configured or 
 *        re-configured when the system settings change.
 */
class iConfigurable {
public:
    virtual ~iConfigurable() = default;

    /**
     * @brief Callback triggered when configuration is initially loaded or 
     *        subsequently updated via the Web UI.
     * @param config The updated configuration state.
     */
    virtual void reapplyConfig(const Config& config) = 0;
};
