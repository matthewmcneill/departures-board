/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * Module: modules/displayManager/boards/systemBoard/diagnosticBoard.cpp
 * Description: Implementation of the hardware diagnostic board.
 */

#include "diagnosticBoard.hpp"
#include <fonts/fonts.hpp>
#include <appContext.hpp>
#include <fonts/fonts.hpp>
#include "layouts/layoutDiagnostic.hpp"

DiagnosticBoard::DiagnosticBoard(appContext* contextPtr) 
    : context(contextPtr), activeLayout(nullptr) {
    activeLayout = new layoutTestDiagnostic(context);
}

DiagnosticBoard::~DiagnosticBoard() {
    if (activeLayout) delete activeLayout;
}

void DiagnosticBoard::onActivate() {
}

void DiagnosticBoard::onDeactivate() {
}

void DiagnosticBoard::tick(uint32_t ms) {
    if (activeLayout) activeLayout->tick(ms);
}

void DiagnosticBoard::render(U8G2& display) {
    if (activeLayout) activeLayout->render(display);
}

void DiagnosticBoard::renderAnimationUpdate(U8G2& display, uint32_t currentMillis) {
    if (activeLayout) activeLayout->renderAnimationUpdate(display, currentMillis);
}

void DiagnosticBoard::configure(const struct BoardConfig& config) {
}
