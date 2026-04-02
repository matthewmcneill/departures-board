/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/boards/systemBoard/wizardBoard.hpp
 * Description: UI Board specifically built to orchestrate the dynamic Wi-Fi setup instructions.
 *
 * Exported Functions/Classes:
 * - WizardBoard: Class extending iDisplayBoard for user provisioning screens.
 */

#ifndef WIZARD_BOARD_HPP
#define WIZARD_BOARD_HPP

class appContext;

#include "../interfaces/iDisplayBoard.hpp"
#include "../../widgets/drawingPrimitives.hpp"
#include <WiFi.h>

/**
 * @brief Board that renders multi-step connection wizard screens.
 */
class WizardBoard : public iDisplayBoard {
private:
    appContext* context;
    IPAddress currentIp;
    uint32_t lastStageSwitch;
    int currentStage; ///< 0=Wizard, 1=Keys Help, 2=CRS Help
    WeatherStatus weatherStatus;

protected:
    WizardBoard();
    friend class DisplayManager;

public:
    const char* getBoardName() const override { return "SYS: Setup Wizard"; }

    /**
     * @brief Injects the IP address string that the user should navigate to.
     * @param ip The active connection IP to display.
     */
    void setWizardIp(IPAddress ip);

    void init(appContext* contextPtr) { context = contextPtr; }

    // iDisplayBoard Implementation
    void onActivate() override;
    void onDeactivate() override;
    void configure(const struct BoardConfig& config) override { (void)config; }
    void tick(uint32_t ms) override;
    void render(U8G2& display) override;
    void renderAnimationUpdate(U8G2& display, uint32_t currentMillis) override { (void)display; (void)currentMillis; }
    UpdateStatus updateData() override { return UpdateStatus::SUCCESS; }
    const char* getLastErrorMsg() override { return ""; }
    WeatherStatus& getWeatherStatus() override { return weatherStatus; }
};

#endif // WIZARD_BOARD_HPP
