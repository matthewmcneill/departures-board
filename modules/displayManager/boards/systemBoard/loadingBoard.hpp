/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/boards/systemBoard/loadingBoard.hpp
 * Description: Implements a boot/loading display featuring the system logo, title,
 *              build time, status text, and a progress bar widget.
 *
 * Exported Functions/Classes:
 * - LoadingBoard: Class extending iDisplayBoard for loading sequences.
 */

#ifndef LOADING_BOARD_HPP
#define LOADING_BOARD_HPP

class appContext;

#include "../interfaces/iDisplayBoard.hpp"
#include "../../widgets/progressBarWidget.hpp"
#include "../../widgets/imageWidget.hpp"
#include "../../widgets/drawingPrimitives.hpp"

/**
 * @brief Display board for the system boot, loading, or configuration sequences.
 */
class LoadingBoard : public iDisplayBoard {
private:
    appContext* context;
    char noticeMessage[64];
    char heading[32];
    char buildTime[32];
    progressBarWidget pBar;
    WeatherStatus weatherStatus;
    
protected:
    LoadingBoard();
    friend class DisplayManager;
    friend class FirmwareUpdateBoard;

public:
    const char* getBoardName() const override { return "SYS: Loading"; }
    
    // Configures the displayed text and progress bar level
    void setProgress(const char* message, int percent, uint32_t durationMs = 0);
    void setHeading(const char* newHeading);
    void setBuildTime(const char* newBuildTime);
    void setNotice(const char* message);

    void init(appContext* contextPtr) { context = contextPtr; }

    // iDisplayBoard Implementation
    void onActivate() override;
    void onDeactivate() override;
    void configure(const struct BoardConfig& config) override { (void)config; }
    void tick(uint32_t ms) override;
    void render(U8G2& display) override;
    int updateData() override { return 0; }
    const char* getLastErrorMsg() override { return ""; }
    void renderAnimationUpdate(U8G2& display, uint32_t currentMillis) override;
    WeatherStatus& getWeatherStatus() override { return weatherStatus; }
};

#endif // LOADING_BOARD_HPP
