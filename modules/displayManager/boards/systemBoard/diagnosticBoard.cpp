/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * Module: modules/displayManager/boards/systemBoard/diagnosticBoard.cpp
 * Description: Implementation of the hardware diagnostic board.
 *
 * Exported Functions/Classes:
 * - DiagnosticBoard: [Class implementation]
 *   - onActivate() / onDeactivate(): Lifecycle management.
 *   - tick() / render(): Dispatch logic to layoutDiagnostic.
 */

#include "diagnosticBoard.hpp"
#include <fonts/fonts.hpp>
#include <appContext.hpp>
#include <fonts/fonts.hpp>
#include "layouts/layoutDiagnostic.hpp"

/**
 * @brief Construct a new Diagnostic Board.
 * @param contextPtr Pointer to shared application context.
 */
DiagnosticBoard::DiagnosticBoard(appContext* contextPtr) 
    : context(contextPtr), activeLayout(nullptr) {
    activeLayout = std::make_unique<layoutTestDiagnostic>(context);
}

DiagnosticBoard::~DiagnosticBoard() {
}

void DiagnosticBoard::onActivate() {
}

/**
 * @brief lifecycle hook for deactivation.
 */
void DiagnosticBoard::onDeactivate() {
}

void DiagnosticBoard::tick(uint32_t ms) {
    if (activeLayout) activeLayout->tick(ms);
}

/**
 * @brief Dispatches the main render call to the diagnostic layout.
 * @param display Reference to the global U8g2 graphics instance.
 */
void DiagnosticBoard::render(U8G2& display) {
    if (activeLayout) activeLayout->render(display);
}

void DiagnosticBoard::renderAnimationUpdate(U8G2& display, uint32_t currentMillis) {
    if (activeLayout) activeLayout->renderAnimationUpdate(display, currentMillis);
}

void DiagnosticBoard::configure(const struct BoardConfig& config) {
}
