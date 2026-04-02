/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/boards/busBoard/busBoard.hpp
 * Description: Controller for TfL Bus departure boards. Manages the 
 *              asynchronous lifecycle of bus stop arrivals using the 
 *              Instant API.
 *
 * Exported Functions/Classes:
 * - BusBoard: Controller class for Bus displays.
 *   - onActivate() / onDeactivate(): Lifecycle hooks for display transitions.
 *   - tick(): Logic update for timing and scrollers.
 *   - render(): Full frame drawing.
 *   - renderAnimationUpdate(): Targeted redraw for animation quality.
 *   - updateData(): Initiates JSON fetch from TfL API.
 *   - configure(): Applies BoardConfig settings to local state.
 *   - getLastErrorMsg(): Accessor for data source error strings.
 *   - getWeatherStatus(): Accessor for shared weather state.
 */

#ifndef BUS_BOARD_HPP
#define BUS_BOARD_HPP

class appContext;

#include "../interfaces/iDisplayBoard.hpp"
#include "busDataSource.hpp"
#include <configManager.hpp>
#include "iBusLayout.hpp"
#include "layouts/layoutDefault.hpp"

/**
 * @brief Represents the physical or logical screen for displaying bus departures.
 * 
 * Manages fetching data via the busDataSource and rendering it using
 * a collection of generic UI widgets encapsulated in a Layout.
 */
class BusBoard : public iDisplayBoard {
private:
    appContext* context;
    busDataSource dataSource; ///< The data source responsible for fetching bus stop data
    
    // UI Layout
    iBusLayout* activeLayout;

    // Configuration
    char busAtco[13];     ///< ATCO code of the bus stop
    char busName[80];     ///< Friendly name of the bus stop (displayed in the header)
    char busFilter[54];   ///< Filter string or constraints
    float busLat, busLon; ///< Coordinates of the bus stop
    bool enableBus;       ///< Flag indicating if the board is enabled

    // Centralized Configuration
    BoardConfig config;

    uint32_t lastUpdate;  ///< Timestamp of the last successful data update
    bool needsRefresh;    ///< Flag indicating if the display requires a complete refresh
    WeatherStatus weatherStatus;

public:
    const char* getBoardName() const override { return "DATA: Bus Board"; }
    /**
     * @brief Constructs a new BusBoard instance.
     */
    BusBoard(appContext* contextPtr = nullptr);
    /**
     * @brief Destroys the BusBoard instance.
     */
    virtual ~BusBoard();

    // iDisplayBoard implementation

    /**
     * @brief Called when the board becomes the active display.
     */
    void onActivate() override;

    /**
     * @brief Called when the board is deactivated.
     */
    void onDeactivate() override;

    /**
     * @brief Core loop processing for logic and data updates.
     * @param ms Current system uptime in milliseconds.
     */
    void tick(uint32_t ms) override;

    /**
     * @brief Renders the board content to the display buffer.
     * @param display The U8G2 display object to render to.
     */
    void render(U8G2& display) override;

    /**
     * @brief Processes animation states, avoiding full redraws where possible.
     * @param display The U8G2 display object for rendering animations.
     * @param currentMillis Current system uptime in milliseconds.
     */
    void renderAnimationUpdate(U8G2& display, uint32_t currentMillis) override;
    void configure(const struct BoardConfig& config) override;
    bool isScrollFinished() override;

    // Configuration Getters/Setters

    /**
     * @brief Sets the ATCO code for the bus stop.
     * @param atco C-string containing the new ATCO code.
     */
    void setBusAtco(const char* atco) { strlcpy(busAtco, atco, sizeof(busAtco)); }

    /**
     * @brief Sets the friendly name of the bus stop.
     * @param name C-string containing the new name.
     */
    void setBusName(const char* name) { strlcpy(busName, name, sizeof(busName)); }

    /**
     * @brief Sets the filter string for bus services.
     * @param filter C-string containing the new filter.
     */
    void setBusFilter(const char* filter) { strlcpy(busFilter, filter, sizeof(busFilter)); }

    /**
     * @brief Sets whether the bus board is enabled and active.
     * @param enable Boolean indicating whether the board should be enabled.
     */
    void setEnableBus(bool enable) { enableBus = enable; }

    /**
     * @brief Sets the geographical latitude.
     * @param lat Floating point latitude value.
     */
    void setBusLat(float lat) { busLat = lat; }

    /**
     * @brief Sets the geographical longitude.
     * @param lon Floating point longitude value.
     */
    void setBusLon(float lon) { busLon = lon; }

    const char* getBusAtco() const { return busAtco; }
    const char* getBusName() const { return busName; }
    const char* getBusFilter() const { return busFilter; }
    bool getEnableBus() const { return enableBus; }
    float getBusLat() const { return busLat; }
    float getBusLon() const { return busLon; }

    /**
     * @brief Triggers an immediate update from the internal data source.
     * @return UpdateStatus on success or error.
     */
    UpdateStatus updateData() override;

    /**
     * @brief Retrieves the last error message from the components.
     * @return C-string representing the specific error description.
     */
    const char* getLastErrorMsg() override { return dataSource.getLastErrorMsg(); }
    WeatherStatus& getWeatherStatus() override { return weatherStatus; }
};

#endif // BUS_BOARD_HPP
