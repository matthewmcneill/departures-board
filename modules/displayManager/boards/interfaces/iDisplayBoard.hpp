/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/boards/interfaces/iDisplayBoard.hpp
 * Description: Core abstract interface for all display boards (screens).
 *
 * Exported Functions/Classes:
 * - iDisplayBoard: [Interface] Primary class for displayable application models.
 *   - getBoardName(): Semantic identity for telemetry.
 *   - onActivate() / onDeactivate(): Lifecycle state transitions.
 *   - tick() / render(): Logic and drawing entry points.
 *   - updateData(): Explicit request to refresh backend data.
 *   - getLastErrorMsg(): Accessor for source error strings.
 *   - getLastUpdateStatus(): Retrieval of fetch result codes.
 *   - configure(): Provision settings from BoardConfig.
 *   - renderAnimationUpdate(): High-speed partial-frame updates.
 *   - getWeatherStatus(): Shared weather state accessor.
 *   - isScrollFinished(): Pagination/scrolling completion check.
 */

#ifndef I_DISPLAY_BOARD_HPP
#define I_DISPLAY_BOARD_HPP

#include <U8g2lib.h>
#include <stdint.h>
#include "../../../dataManager/iDataSource.hpp"
#include <weatherStatus.hpp>

/**
 * @brief Pure virtual interface representing a displayable screen.
 *        Follows the state pattern for UI lifecycle management.
 */
class iDisplayBoard {
public:
    /**
     * @brief Self-identifying semantic name for lifecycle logging telemetry.
     */
    virtual const char* getBoardName() const = 0;
    /**
     * @brief Virtual destructor.
     */
    virtual ~iDisplayBoard() = default;

    /**
     * @brief Called when the board becomes the active display.
     */
    virtual void onActivate() = 0;

    /**
     * @brief Called when the board is no longer the active display.
     */
    virtual void onDeactivate() = 0;

    /**
     * @brief Main logic tick called periodically by the main loop.
     *        Use this for standard state mutations and timing checks.
     *        This safely precedes the synchronized full-screen `render()` pass.
     * @param ms Current system time in milliseconds.
     */
    virtual void tick(uint32_t ms) = 0;

    /**
     * @brief Main rendering hook called when the display needs to be drawn.
     * @param display Reference to the global U8g2 object.
     */
    virtual void render(U8G2& display) = 0;

    /**
     * @brief Forces an immediate data update.
     * @return Update status code.
     */
    virtual UpdateStatus updateData() = 0;

    /**
     * @brief Retrieves the last error message from the board's data source.
     * @return A string containing the error.
     */
    virtual const char* getLastErrorMsg() = 0;

    /**
     * @brief Retrieves the HTTP or internal result code from the last updateData() call.
     * @return Result code (e.g., UpdateStatus::SUCCESS).
     */
    virtual UpdateStatus getLastUpdateStatus() const { return lastUpdateStatus; }

    /**
     * @brief Apply a specific configuration to this board.
     * @param config The BoardConfig struct containing settings for this instance.
     */
    virtual void configure(const struct BoardConfig& config) = 0;

    /**
     * @brief Emergency non-blocking hardware-accelerated propagation pipeline.
     *        ONLY fire this during long blocking operations (e.g. HTTP fetches).
     *        Implementations must propagate this call to widgets requiring 
     *        localized partial-screen SPI updates (e.g. clocks, scrollers).
     *        Widgets should implement the "Deduplication Pattern" (see iGfxWidget).
     * @param display Reference to the global U8g2 instance.
     * @param currentMillis Current system time in milliseconds.
     */
    virtual void renderAnimationUpdate(U8G2& display, uint32_t currentMillis) {}

    /**
     * @brief Access the weather status associated with this board.
     * @return Reference to the WeatherStatus object.
     */
    virtual class WeatherStatus& getWeatherStatus() = 0;

    /**
     * @brief Check if the board has finished its current display cycle (e.g. scrolling).
     *        Used by DisplayManager to pace carousel transitions.
     * @return True if the board is ready to be swapped.
     */
    virtual bool isScrollFinished() { return true; }

protected:
    UpdateStatus lastUpdateStatus = UpdateStatus::NO_DATA; ///< Stores the result of the last updateData() call for inline error handling.
    int consecutiveErrors = 0; ///< Tracks the number of consecutive data fetch failures to defer error overlays.
};

#endif // I_DISPLAY_BOARD_HPP
