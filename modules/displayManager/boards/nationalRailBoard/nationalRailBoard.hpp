/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/boards/nationalRailBoard/nationalRailBoard.hpp
 * Description: Controller for the National Rail departure board. Manages the 
 *              lifecycle of the NR data source, coordinates with the UI layout, 
 *              and handles station-specific configuration (CRS codes, tokens, filters).
 *
 * Exported Functions/Classes:
 * - NationalRailBoard: Core controller class for NR displays.
 *   - onActivate() / onDeactivate(): Lifecycle hooks for display transitions.
 *   - tick(): Logic update for timing and toggles.
 *   - render(): Full frame drawing.
 *   - renderAnimationUpdate(): Targeted redraw for animation quality.
 *   - updateData(): Triggers asynchronous SOAP fetch from OpenLDBWS.
 *   - configure(): Applies BoardConfig settings to local state.
 *   - getLastErrorMsg(): Accessor for data source error strings.
 *   - getWeatherStatus(): Accessor for shared weather state.
 */

#ifndef NATIONAL_RAIL_BOARD_HPP
#define NATIONAL_RAIL_BOARD_HPP

class appContext;

#include "../interfaces/iDisplayBoard.hpp"
#include "nationalRailDataSource.hpp"
#include <configManager.hpp>
#include "iNationalRailLayout.hpp"
#include "layouts/layoutDefault.hpp"

class NationalRailBoard : public iDisplayBoard {
private:
    appContext* context;
    nationalRailDataSource dataSource;
    
    // UI Layout
    iNationalRailLayout* activeLayout;

    // Configuration
    char nrToken[37];
    char crsCode[4];
    float stationLat, stationLon;
    int nrTimeOffset;
    char callingCrsCode[4];
    char callingStation[45];
    char platformFilter[54];

    // Centralized Configuration
    BoardConfig config;

    uint32_t lastUpdate;
    bool viaToggle;
    uint32_t nextViaToggle;
    
    char cachedOrdinals[16][8];
    char firstOrdinal[4];
    WeatherStatus weatherStatus;

public:
    const char* getBoardName() const override { return "DATA: National Rail"; }
    /**
     * @brief Construct a new National Rail Board instance.
     * @param contextPtr Pointer to the global application context.
     */
    NationalRailBoard(appContext* contextPtr = nullptr);

    /**
     * @brief Cleanup allocated resources and layouts.
     */
    virtual ~NationalRailBoard();

    // iDisplayBoard implementation
    void onActivate() override;
    void onDeactivate() override;
    void tick(uint32_t ms) override;
    void render(U8G2& display) override;
    void renderAnimationUpdate(U8G2& display, uint32_t currentMillis) override;
    void configure(const struct BoardConfig& config) override;
    bool isScrollFinished() override;

    // Configuration Getters/Setters
    /**
     * @brief Set the OpenLDBWS API access token.
     * @param token C-string token.
     */
    void setNrToken(const char* token) { strlcpy(nrToken, token, sizeof(nrToken)); }

    /**
     * @brief Set the destination/origin station CRS code.
     * @param code 3-letter CRS code.
     */
    void setCrsCode(const char* code) { strlcpy(crsCode, code, sizeof(crsCode)); }

    /**
     * @brief Offset the local time shown on the board.
     * @param offset Minutes to offset.
     */
    void setNrTimeOffset(int offset) { nrTimeOffset = offset; }

    /**
     * @brief Set the filter for calling point CRS codes.
     * @param code 3-letter CRS code.
     */
    void setCallingCrsCode(const char* code) { strlcpy(callingCrsCode, code, sizeof(callingCrsCode)); }

    /**
     * @brief Set the human-readable calling station name.
     * @param station Name string.
     */
    void setCallingStation(const char* station) { strlcpy(callingStation, station, sizeof(callingStation)); }

    /**
     * @brief Set the platform filter string.
     * @param filter Filter string (e.g. "1,2").
     */
    void setPlatformFilter(const char* filter) { strlcpy(platformFilter, filter, sizeof(platformFilter)); }

    /**
     * @brief Set the station latitude for weather localizing.
     * @param lat Latitude float.
     */
    void setStationLat(float lat) { stationLat = lat; }

    /**
     * @brief Set the station longitude for weather localizing.
     * @param lon Longitude float.
     */
    void setStationLon(float lon) { stationLon = lon; }

    /** @return Current API token */
    const char* getNrToken() const { return nrToken; }
    /** @return Primary station CRS code */
    const char* getCrsCode() const { return crsCode; }
    /** @return Secondary calling point CRS code */
    const char* getCallingCrsCode() const { return callingCrsCode; }
    /** @return Station latitude */
    float getStationLat() const { return stationLat; }
    /** @return Station longitude */
    float getStationLon() const { return stationLon; }

    /**
     * @brief Trigger an asynchronous data refresh.
     * @return UPD_SUCCESS, UPD_PENDING, or error code.
     */
    int updateData() override;

    /**
     * @brief Populates the layout widgets with the latest data slices.
     */
    void populateServices();

    /**
     * @brief Get the last error string from the data source.
     * @return Error message pointer.
     */
    const char* getLastErrorMsg() override { return dataSource.getLastErrorMsg(); }

    /**
     * @brief Access the shared weather state for this board.
     * @return WeatherStatus reference.
     */
    WeatherStatus& getWeatherStatus() override { return weatherStatus; }
};

#endif // NATIONAL_RAIL_BOARD_HPP
