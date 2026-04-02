/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/boards/systemBoard/splashBoard.hpp
 * Description: Display board for rendering the initial system boot logo and copyright information.
 *
 * Exported Functions/Classes:
 * - SplashBoard: Class extending iDisplayBoard for the initial splash sequence.
 */

#ifndef SPLASH_BOARD_HPP
#define SPLASH_BOARD_HPP

class appContext;

#include "../interfaces/iDisplayBoard.hpp"
#include <memory>
#include "../../widgets/imageWidget.hpp"
#include "../../widgets/progressBarWidget.hpp"
#include "../../widgets/drawingPrimitives.hpp"
#include <string.h>
#include <gfx/xbmgfx.h>

/**
 * @brief Board that renders the 5-second initial startup logo and copyright display.
 */
class SplashBoard : public iDisplayBoard {
private:
    appContext* context;
    char noticeMessage[64];
    std::unique_ptr<imageWidget> splashLogo;
    WeatherStatus weatherStatus;

protected:
    SplashBoard();
    friend class DisplayManager;

public:
    const char* getBoardName() const override { return "SYS: Boot Splash"; }
    ~SplashBoard();

    /**
     * @brief Injects the textual notice displayed below the logo
     * @param message Text string (e.g. Copyright notice)
     */
    void setNotice(const char* message);

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

#endif // SPLASH_BOARD_HPP
