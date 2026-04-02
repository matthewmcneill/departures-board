/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/boardFactory.hpp
 * Description: Factory module to isolate concrete board implementations from the global
 *              DisplayManager orchestrator. Responsible for heap allocation of requested
 *              board classes based on System/User ID.
 *
 * Exported Functions/Classes:
 * - BoardFactory: Factory class for instantiating concrete display boards.
 *   - createDisplayBoard(): Allocates an active polling data board onto the heap.
 *   - createSystemBoard(): Allocates a system overlay board onto the heap.
 */

#pragma once

#include "boards/interfaces/iDisplayBoard.hpp"

class appContext;

class BoardFactory {
public:
    /**
     * @brief Allocates an active polling data board onto the heap.
     * @param type Identifier for the specific board type to instantiate.
     * @param context Dependency injection for the global appContext.
     * @return iDisplayBoard* Pointer to newly allocated board, or nullptr if unknown.
     */
    static iDisplayBoard* createDisplayBoard(int type, appContext* context);

    /**
     * @brief Allocates a system overlay board onto the heap.
     * @param id Identifier for the specific system board to instantiate.
     * @param context Dependency injection for the global appContext.
     * @return iDisplayBoard* Pointer to newly allocated board, or nullptr if unknown.
     */
    static iDisplayBoard* createSystemBoard(int id, appContext* context);
};
