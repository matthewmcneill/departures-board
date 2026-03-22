/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/boards/interfaces/iBoardLayout.hpp
 * Description: Base abstract interface for all visual board layouts (MVC/LGV pattern). 
 *              Decouples coordinate math and rendering from the board controller, 
 *              allowing for multiple design skins (e.g. Default vs Replica) per board.
 *
 * Exported Functions/Classes:
 * - iBoardLayout: Interface for visual designers and layout templates.
 *   - iBoardLayout(): Constructor with context injection.
 *   - tick(): View-specific timing logic (e.g. scrollers).
 *   - render(): Full-frame rendering pass.
 *   - renderAnimationUpdate(): Targeted sub-frame animation updates.
 */

#pragma once
#ifndef I_BOARD_LAYOUT_HPP
#define I_BOARD_LAYOUT_HPP

#include <U8g2lib.h>
#include <stdint.h>
class appContext;

/**
 * @brief Pure virtual interface representing a visual layout (View).
 *        Decouples the rendering and coordinate math from the Board (Controller).
 */
class iBoardLayout {
protected:
    appContext* context;

public:
    /**
     * @brief Constructor for the view.
     * @param _context Pointer to the application context for DI access.
     */
    iBoardLayout(appContext* _context) : context(_context) {}

    /**
     * @brief Virtual destructor.
     */
    virtual ~iBoardLayout() = default;

    /**
     * @brief Periodic logic update for view-specific timing (e.g. scrolling).
     * @param currentMillis Current system time in milliseconds.
     */
    virtual void tick(uint32_t currentMillis) = 0;

    /**
     * @brief Main rendering hook for the entire view.
     * @param display Reference to the global U8g2 instance.
     */
    virtual void render(U8G2& display) = 0;

    /**
     * @brief High-speed non-blocking animation update hook.
     * @param display Reference to the global U8g2 instance.
     * @param currentMillis Current system time in milliseconds.
     */
    virtual void renderAnimationUpdate(U8G2& display, uint32_t currentMillis) {}
};

#endif // I_BOARD_LAYOUT_HPP
