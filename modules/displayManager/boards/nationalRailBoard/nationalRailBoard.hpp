/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/boards/nationalRailBoard/nationalRailBoard.hpp
 * Description: Implementation of iDisplayBoard for National Rail departure boards.
 */

#ifndef NATIONAL_RAIL_BOARD_HPP
#define NATIONAL_RAIL_BOARD_HPP

class appContext;

#include "../interfaces/iDisplayBoard.hpp"
#include "nationalRailDataSource.hpp"
#include "../../widgets/headerWidget.hpp"
#include "../../widgets/clockWidget.hpp"
#include "../../widgets/serviceListWidget.hpp"
#include "../../widgets/scrollingMessagePoolWidget.hpp"

class NationalRailBoard : public iDisplayBoard {
private:
    appContext* context;
    nationalRailDataSource dataSource;
    
    // UI Widgets
    headerWidget headWidget;
    serviceListWidget servicesWidget; // For secondary services
    scrollingMessagePoolWidget msgWidget;

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

    // Alt station support 
    bool altStationEnabled;
    byte altStarts, altEnds;
    char altCrsCode[4];
    float altLat, altLon;
    char altCallingCrsCode[4];
    char altCallingStation[45];
    char altPlatformFilter[54];
    bool altStationActive;

    uint32_t lastUpdate;
    bool viaToggle;
    uint32_t nextViaToggle;
    
    char cachedOrdinals[16][8];
    WeatherStatus weatherStatus;

public:
    NationalRailBoard(appContext* contextPtr = nullptr);
    virtual ~NationalRailBoard() = default;

    // iDisplayBoard implementation
    void onActivate() override;
    void onDeactivate() override;
    void tick(uint32_t ms) override;
    void render(U8G2& display) override;
    void renderAnimationUpdate(U8G2& display, uint32_t currentMillis) override;
    void configure(const struct BoardConfig& config) override;

    // Configuration Getters/Setters
    void setNrToken(const char* token) { strlcpy(nrToken, token, sizeof(nrToken)); }
    void setCrsCode(const char* code) { strlcpy(crsCode, code, sizeof(crsCode)); }
    void setNrTimeOffset(int offset) { nrTimeOffset = offset; }
    void setCallingCrsCode(const char* code) { strlcpy(callingCrsCode, code, sizeof(callingCrsCode)); }
    void setCallingStation(const char* station) { strlcpy(callingStation, station, sizeof(callingStation)); }
    void setPlatformFilter(const char* filter) { strlcpy(platformFilter, filter, sizeof(platformFilter)); }
    void setStationLat(float lat) { stationLat = lat; }
    void setStationLon(float lon) { stationLon = lon; }

    const char* getNrToken() const { return nrToken; }
    const char* getCrsCode() const { return crsCode; }
    const char* getCallingCrsCode() const { return callingCrsCode; }
    float getStationLat() const { return stationLat; }
    float getStationLon() const { return stationLon; }

    // Alt station support
    void setAltStationEnabled(bool enabled) { altStationEnabled = enabled; }
    void setAltStarts(byte starts) { altStarts = starts; }
    void setAltEnds(byte ends) { altEnds = ends; }
    void setAltCrsCode(const char* code) { strlcpy(altCrsCode, code, sizeof(altCrsCode)); }
    void setAltLat(float lat) { altLat = lat; }
    void setAltLon(float lon) { altLon = lon; }
    void setAltCallingCrsCode(const char* code) { strlcpy(altCallingCrsCode, code, sizeof(altCallingCrsCode)); }
    void setAltCallingStation(const char* station) { strlcpy(altCallingStation, station, sizeof(altCallingStation)); }
    void setAltPlatformFilter(const char* filter) { strlcpy(altPlatformFilter, filter, sizeof(altPlatformFilter)); }
    void setAltStationActive(bool active) { altStationActive = active; }

    bool getAltStationEnabled() const { return altStationEnabled; }
    byte getAltStarts() const { return altStarts; }
    byte getAltEnds() const { return altEnds; }
    const char* getAltCrsCode() const { return altCrsCode; }
    float getAltLat() const { return altLat; }
    float getAltLon() const { return altLon; }
    const char* getAltCallingCrsCode() const { return altCallingCrsCode; }
    const char* getAltCallingStation() const { return altCallingStation; }
    const char* getAltPlatformFilter() const { return altPlatformFilter; }
    bool getAltStationActive() const { return altStationActive; }

    int updateData() override;
    const char* getLastErrorMsg() override { return dataSource.getLastErrorMsg(); }
    WeatherStatus& getWeatherStatus() override { return weatherStatus; }
};

#endif // NATIONAL_RAIL_BOARD_HPP
