/*
 * Departures Board (c) 2025-2026 Gadec Software
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

#include "../interfaces/iDisplayBoard.hpp"
#include "../../widgets/imageWidget.hpp"
#include "../../widgets/drawingPrimitives.hpp"
#include <string.h>
#include "../../../../../include/gfx/xbmgfx.h"

/**
 * @brief Board that renders the 5-second initial startup logo and copyright display.
 */
class SplashBoard : public iDisplayBoard {
private:
    char noticeMessage[64];
    imageWidget* splashLogo;

protected:
    SplashBoard();
    friend class DisplayManager;

public:
    ~SplashBoard();

    /**
     * @brief Injects the textual notice displayed below the logo
     * @param message Text string (e.g. Copyright notice)
     */
    void setNotice(const char* message);

    // iDisplayBoard Implementation
    void onActivate() override;
    void onDeactivate() override;
    void tick(uint32_t ms) override;
    void render(U8G2& display) override;
    int updateData() override { return 0; }
    const char* getLastErrorMsg() override { return ""; }
};

#endif // SPLASH_BOARD_HPP
