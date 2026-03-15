/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/boards/interfaces/iDisplayBoard.hpp
 * Description: Interface for all display boards (screens).
 */

#ifndef I_DISPLAY_BOARD_HPP
#define I_DISPLAY_BOARD_HPP

#include <U8g2lib.h>
#include <stdint.h>
#include <weatherStatus.hpp>

/**
 * @brief Pure virtual interface representing a displayable screen.
 *        Follows the state pattern for UI lifecycle management.
 */
class iDisplayBoard {
public:
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
    virtual int updateData() = 0;

    /**
     * @brief Retrieves the last error message from the board's data source.
     * @return A string containing the error.
     */
    virtual const char* getLastErrorMsg() = 0;

    /**
     * @brief Retrieves the HTTP or internal result code from the last updateData() call.
     * @return Result code (e.g., 0 for Success, 5 for Unauthorised).
     */
    virtual int getLastUpdateStatus() const { return lastUpdateStatus; }

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

protected:
    int lastUpdateStatus = -1; ///< Stores the result of the last updateData() call for inline error handling.
};

#endif // I_DISPLAY_BOARD_HPP
