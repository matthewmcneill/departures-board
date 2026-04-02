/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * Module: modules/displayManager/boards/systemBoard/diagnosticBoard.hpp
 * Description: Controller for the hardware diagnostic and calibration board.
 *              Renders a coordinate grid and alignment boxes.
 */

#ifndef DIAGNOSTIC_BOARD_HPP
#define DIAGNOSTIC_BOARD_HPP

#include <memory>
#include "../interfaces/iDisplayBoard.hpp"
#include "layouts/layoutDiagnostic.hpp"

class appContext;

class DiagnosticBoard : public iDisplayBoard {
private:
    appContext* context;
    std::unique_ptr<layoutTestDiagnostic> activeLayout;
    WeatherStatus weather;

public:
    const char* getBoardName() const override { return "SYS: Diagnostics"; }
    DiagnosticBoard(appContext* contextPtr = nullptr);
    virtual ~DiagnosticBoard();

    void init(appContext* contextPtr) { context = contextPtr; }

    void onActivate() override;
    void onDeactivate() override;
    void tick(uint32_t ms) override;
    void render(U8G2& display) override;
    void renderAnimationUpdate(U8G2& display, uint32_t currentMillis) override;
    void configure(const struct BoardConfig& config) override;
    const char* getLastErrorMsg() override { return nullptr; }
    UpdateStatus updateData() override { return UpdateStatus::SUCCESS; }
    WeatherStatus& getWeatherStatus() override { return weather; }
};

#endif // DIAGNOSTIC_BOARD_HPP
