/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
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

class appContext;

#include "../interfaces/iDisplayBoard.hpp"
#include "../../widgets/drawingPrimitives.hpp"

/**
 * @brief Board that renders stacked lines of instructional text.
 */
class HelpBoard : public iDisplayBoard {
private:
    appContext* context;
    char title[32];
    const char* lines[5];
    int lineCount;
    WeatherStatus weatherStatus;

protected:
    HelpBoard();
    friend class DisplayManager;

public:
    const char* getBoardName() const override { return "SYS: Setup Help"; }

    /**
     * @brief Injects the stacked string array.
     * @param h The centered top header.
     * @param textArr Array of string pointers.
     * @param count Number of lines in the array (max 5).
     */
    void setHelpContent(const char* h, const char* textArr[], int count);

    void init(appContext* contextPtr) { context = contextPtr; }

    // iDisplayBoard Implementation
    void onActivate() override;
    void onDeactivate() override;
    void configure(const struct BoardConfig& config) override { (void)config; }
    void tick(uint32_t ms) override;
    void render(U8G2& display) override;
    UpdateStatus updateData() override { return UpdateStatus::SUCCESS; }
    const char* getLastErrorMsg() override { return ""; }
    WeatherStatus& getWeatherStatus() override { return weatherStatus; }
};

#endif // HELP_BOARD_HPP
