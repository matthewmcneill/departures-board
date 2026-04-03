/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/boardFactory.cpp
 * Description: Factory module to isolate concrete board implementations.
 *
 * Exported Functions/Classes:
 * - BoardFactory: Factory class for instantiating concrete display boards.
 *   - createDisplayBoard(): Allocates an active polling data board onto the heap.
 *   - createSystemBoard(): Allocates a system overlay board onto the heap.
 */
#include "boardFactory.hpp"
#include "displayManager.hpp"
#include <appContext.hpp>

// Concrete Includes
#include "boards/nationalRailBoard/nationalRailBoard.hpp"
#include "boards/tflBoard/tflBoard.hpp"
#include "boards/busBoard/busBoard.hpp"
#include "boards/systemBoard/diagnosticBoard.hpp"

#include "boards/systemBoard/splashBoard.hpp"
#include "boards/systemBoard/loadingBoard.hpp"
#include "boards/systemBoard/wizardBoard.hpp"
#include "boards/systemBoard/helpBoard.hpp"
#include "boards/systemBoard/messageBoard.hpp"
#include "boards/systemBoard/firmwareUpdateBoard.hpp"
#include "boards/systemBoard/sleepingBoard.hpp"
#include <logger.hpp>


iDisplayBoard* BoardFactory::createDisplayBoard(int type, appContext* context) {
    BoardType boardType = static_cast<BoardType>(type);

    try {
        switch (boardType) {
            case BoardType::NR_BOARD:
                return new NationalRailBoard(context);
            case BoardType::TFL_BOARD:
                return new TfLBoard(context);
            case BoardType::BUS_BOARD:
                return new BusBoard(context);
            case BoardType::CLOCK_BOARD:
                return new SleepingBoard(context);
            case BoardType::DIAGNOSTIC_BOARD:
                return new DiagnosticBoard(context);
            default:
                return nullptr;
        }
    } catch (const std::bad_alloc& e) {
        LOG_ERROR("SYSTEM", "CRITICAL OOM: Failed to allocate active board on heap!");
        return nullptr;
    }
}

iDisplayBoard* BoardFactory::createSystemBoard(int id, appContext* context) {
    SystemBoardId boardId = static_cast<SystemBoardId>(id);
    
    // Note: To match the static initialization pattern of default constructors
    // before the DisplayManager begin() injects context, we use the default
    // constructor and let the caller invoke init(context). If the board
    // supports setting context in constructor, we should do that, but to 
    // restrict changes to just the displayManager.cpp we will keep it simple.
    
    iDisplayBoard* newBoard = nullptr;

    try {
        switch (boardId) {
            case SystemBoardId::SYS_BOOT_SPLASH:
                newBoard = new SplashBoard();
                break;
            case SystemBoardId::SYS_BOOT_LOADING:
                newBoard = new LoadingBoard();
                break;
            case SystemBoardId::SYS_WIFI_WIZARD:
                newBoard = new WizardBoard();
                break;
            case SystemBoardId::SYS_SETUP_HELP:
                newBoard = new HelpBoard();
                break;
            case SystemBoardId::SYS_ERROR_NO_DATA:
            case SystemBoardId::SYS_ERROR_WSDL:
            case SystemBoardId::SYS_ERROR_TOKEN:
            case SystemBoardId::SYS_ERROR_CRS:
            case SystemBoardId::SYS_ERROR_WIFI:
                newBoard = new MessageBoard();
                break;
            case SystemBoardId::SYS_FIRMWARE_UPDATE:
                newBoard = new FirmwareUpdateBoard();
                break;
            case SystemBoardId::SYS_SLEEP_CLOCK:
                newBoard = new SleepingBoard(context); // Sleeping board takes context in constructor
                break;
            case SystemBoardId::SYS_DIAGNOSTIC:
                newBoard = new DiagnosticBoard(context); // Diagnostic board takes context
                break;
            default:
                return nullptr;
        }
    } catch (const std::bad_alloc& e) {
        LOG_ERROR("SYSTEM", "CRITICAL OOM: Failed to allocate system board on heap!");
        return nullptr;
    }
    
    return newBoard;
}
