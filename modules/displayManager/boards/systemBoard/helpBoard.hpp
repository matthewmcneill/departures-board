/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/boards/systemBoard/helpBoard.hpp
 * Description: UI Board specifically built to orchestrate paginated instruction flows.
 *
 * Exported Functions/Classes:
 * - HelpBoard: Class extending iDisplayBoard for user provisioning screens.
 */

#ifndef HELP_BOARD_HPP
#define HELP_BOARD_HPP

#include "../interfaces/iDisplayBoard.hpp"
#include "../../widgets/drawingPrimitives.hpp"

/**
 * @brief Board that renders stacked lines of instructional text.
 */
class HelpBoard : public iDisplayBoard {
private:
    char title[32];
    const char* lines[5];
    int lineCount;

protected:
    HelpBoard();
    friend class DisplayManager;

public:

    /**
     * @brief Injects the stacked string array.
     * @param h The centered top header.
     * @param textArr Array of string pointers.
     * @param count Number of lines in the array (max 5).
     */
    void setHelpContent(const char* h, const char* textArr[], int count);

    // iDisplayBoard Implementation
    void onActivate() override;
    void onDeactivate() override;
    void tick(uint32_t ms) override;
    void render(U8G2& display) override;
    int updateData() override { return 0; }
    const char* getLastErrorMsg() override { return ""; }
};

#endif // HELP_BOARD_HPP
