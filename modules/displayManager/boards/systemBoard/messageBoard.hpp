/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/boards/systemBoard/messageBoard.hpp
 * Description: Generic screen for formatting arbitrary error headers, bold titles,
 *              and multi-line subtitular content. Used heavily for system alerts.
 *
 * Exported Functions/Classes:
 * - MessageBoard: Class extending iDisplayBoard to handle error/alert layouts.
 */

#ifndef MESSAGE_BOARD_HPP
#define MESSAGE_BOARD_HPP

#include "../interfaces/iDisplayBoard.hpp"
#include "../../widgets/drawingPrimitives.hpp"
#include <string.h>

/**
 * @brief Board that safely presents configurable 3-layer alert messages to the user.
 */
class MessageBoard : public iDisplayBoard {
private:
    char hdr[32];
    char title[32];
    char msg1[48];
    char msg2[48];
    char msg3[48];
    char msg4[48];
    bool showWarningIcon;

protected:
    MessageBoard();
    friend class DisplayManager;
    friend class FirmwareUpdateBoard;

public:

    /**
     * @brief Configure the text currently displayed on the warning screen.
     * @param h The small uppermost header (e.g. "** COMMS ERROR **")
     * @param t The large bold title (e.g. "NO DEPARTURE DATA")
     * @param m1 Body line 1
     * @param m2 Body line 2
     * @param m3 Body line 3
     * @param m4 Body line 4
     */
    void setContent(const char* h, const char* t, 
                    const char* m1 = "", const char* m2 = "", 
                    const char* m3 = "", const char* m4 = "");

    void setWarningIcon(bool show);

    // iDisplayBoard Implementation
    void onActivate() override;
    void onDeactivate() override;
    void tick(uint32_t ms) override;
    void render(U8G2& display) override;
    int updateData() override { return 0; }
    const char* getLastErrorMsg() override { return ""; }
};

#endif // MESSAGE_BOARD_HPP
