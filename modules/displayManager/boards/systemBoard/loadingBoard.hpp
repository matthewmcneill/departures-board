/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/boards/systemBoard/loadingBoard.hpp
 * Description: Renders the boot-time status screen. Provides a central 
 *              branding logo, build metadata, and a technical progress 
 *              indicator for firmware init and connection sequences.
 *
 * Exported Functions/Classes:
 * - LoadingBoard: System board for startup sequences.
 *   - setProgress(): Update the active task and percentage.
 *   - setHeading(): Set the primary display title (e.g. "Initializing").
 *   - setBuildTime(): Display the compiled binary timestamp.
 *   - setNotice(): Set a secondary status string.
 *   - init(): Dependency injection for app context.
 *   - tick(): Logic update for progress bar.
 *   - render(): Full frame drawing.
 *   - renderAnimationUpdate(): Targeted redraw for smooth progress.
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
    /**
     * @brief Set a secondary status notification.
     * @param message Status text.
     */
    void setNotice(const char* message);

    /**
     * @brief Inject the application context.
     * @param contextPtr Pointer to context.
     */
    void init(appContext* contextPtr) { context = contextPtr; }

    // iDisplayBoard Implementation
    /**
     * @brief Lifecycle hook for board entry.
     */
    void onActivate() override;

    /**
     * @brief Lifecycle hook for board exit.
     */
    void onDeactivate() override;

    /**
     * @brief No-op configuration for system boards.
     */
    void configure(const struct BoardConfig& config) override { (void)config; }

    /**
     * @brief Logic pulse for the internal progress bar.
     * @param ms Milliseconds since boot.
     */
    void tick(uint32_t ms) override;

    /**
     * @brief Primary render for the logo and progress state.
     * @param display U8g2 reference.
     */
    void render(U8G2& display) override;

    /** @return Always 0 for system boards. */
    UpdateStatus updateData() override { return UpdateStatus::SUCCESS; }

    /** @return Empty string for system boards. */
    const char* getLastErrorMsg() override { return ""; }

    /**
     * @brief Optimization for progress bar movement.
     * @param display U8g2 reference.
     * @param currentMillis Milliseconds since boot.
     */
    void renderAnimationUpdate(U8G2& display, uint32_t currentMillis) override;

    /**
     * @brief Access the shared weather state (unused on loading).
     * @return WeatherStatus reference.
     */
    WeatherStatus& getWeatherStatus() override { return weatherStatus; }
};

#endif // LOADING_BOARD_HPP
