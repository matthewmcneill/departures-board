/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/boards/nationalRailBoard/include/nationalRailBoard.hpp
 * Description: Implementation of the IStation interface for 
 *              National Rail departure boards. Handles the specific visual 
 *              rendering of train services and station headers.
 *
 * Provides:
 * - NationalRailService: Data representation of a National Rail train departure.
 * - NationalRailBoard: Concrete implementation of IStation for National Rail.
 */

#ifndef NATIONAL_RAIL_BOARD_HPP
#define NATIONAL_RAIL_BOARD_HPP

#include "../../interfaces/IStation.hpp"
#include "raildataXmlClient.hpp"
#include "../../interfaces/messageData.h"



// Shared constants for service types
#define NR_SERVICE_OTHER 0 // Unidentified or other service type
#define NR_SERVICE_TRAIN 1 // Standard passenger train service
#define NR_SERVICE_BUS 2   // Rail replacement bus service

/**
 * @brief Data structure representing a single train service or bus replacement arrival.
 *        Stores all the physical attributes needed to render the service onto the OLED display.
 */
struct NationalRailService {
    char sTime[6];
    char destination[NR_MAX_LOCATION];
    char via[NR_MAX_LOCATION];
    char etd[11];
    char platform[4];
    bool isCancelled;
    bool isDelayed;
    int trainLength;
    byte classesAvailable;
    char opco[50];
    char calling[NR_MAX_CALLING];
    char origin[NR_MAX_LOCATION];
    char serviceMessage[NR_MAX_MSG_LEN];
    int serviceType; // TRAIN or BUS
};

/**
 * @brief Top-level data entity wrapping the localized station variables and an array
 *        of the upcoming departures from the National Rail network.
 */
struct NationalRailStation {
    char location[NR_MAX_LOCATION];
    bool platformAvailable;
    bool boardChanged;
    int numServices;
    NationalRailService service[NR_MAX_SERVICES];
};

/**
 * @brief Concrete implementation of IStation for National Rail departure boards.
 *        Implements the specific drawing and scrolling logic for train services.
 */
class NationalRailBoard : public IStation {
private:
    NationalRailStation stationData;
    char lastErrorMsg[128] = "";
    char serviceMessage[NR_MAX_MSG_LEN];

    // --- Animation & Rendering State ---
    uint32_t timer = 0;
    int numMessages = 0;
    int scrollStopsXpos = 0;
    int scrollStopsYpos = 0;
    int scrollStopsLength = 0;
    bool isScrollingStops = false;
    int currentMessage = 0;
    int prevMessage = 0;
    int prevScrollStopsLength = 0;
    
    // Max 10 messages (service messages + calling points) of up to 400 chars each.
    char line2[10][NR_MAX_MSG_LEN];
    uint32_t viaTimer = 0;
    bool isShowingVia = false;
    
    uint32_t serviceTimer = 0;
    bool isScrollingService = false;
    int line3Service = 0;
    int scrollServiceYpos = 0;
    int prevService = 0;
    
    // --- Board Configuration Variables ---
    char nrToken[37] = "";
    char crsCode[4] = "";
    float stationLat = 0;
    float stationLon = 0;
    int nrTimeOffset = 0;

    char callingCrsCode[4] = "";
    char callingStation[45] = "";
    char platformFilter[54] = "";
    char cleanPlatformFilter[54] = "";

    bool altStationEnabled = false;
    byte altStarts = 12;
    byte altEnds = 23;
    char altCrsCode[4] = "";
    float altLat = 0;
    float altLon = 0;
    char altCallingCrsCode[4] = "";
    char altCallingStation[45] = "";
    char altPlatformFilter[54] = "";
    bool altStationActive = false;

public:
    NationalRailBoard() {
        memset(&stationData, 0, sizeof(NationalRailStation));
    }

    // --- Configuration Getters/Setters ---
    const char* getNrToken() const { return nrToken; }
    void setNrToken(const char* token) { strlcpy(nrToken, token, sizeof(nrToken)); }

    const char* getCrsCode() const { return crsCode; }
    void setCrsCode(const char* targetCode) { strlcpy(crsCode, targetCode, sizeof(crsCode)); }

    float getStationLat() const { return stationLat; }
    void setStationLat(float lat) { stationLat = lat; }

    float getStationLon() const { return stationLon; }
    void setStationLon(float lon) { stationLon = lon; }

    int getNrTimeOffset() const { return nrTimeOffset; }
    void setNrTimeOffset(int offset) { nrTimeOffset = offset; }

    const char* getCallingCrsCode() const { return callingCrsCode; }
    void setCallingCrsCode(const char* code) { strlcpy(callingCrsCode, code, sizeof(callingCrsCode)); }

    const char* getCallingStation() const { return callingStation; }
    void setCallingStation(const char* station) { strlcpy(callingStation, station, sizeof(callingStation)); }

    const char* getPlatformFilter() const { return platformFilter; }
    void setPlatformFilter(const char* filter) { strlcpy(platformFilter, filter, sizeof(platformFilter)); }

    const char* getCleanPlatformFilter() const { return cleanPlatformFilter; }
    void setCleanPlatformFilter(const char* filter) { strlcpy(cleanPlatformFilter, filter, sizeof(cleanPlatformFilter)); }

    bool getAltStationEnabled() const { return altStationEnabled; }
    void setAltStationEnabled(bool enabled) { altStationEnabled = enabled; }

    bool getAltStationActive() const { return altStationActive; }
    void setAltStationActive(bool active) { altStationActive = active; }

    byte getAltStarts() const { return altStarts; }
    void setAltStarts(byte starts) { altStarts = starts; }

    byte getAltEnds() const { return altEnds; }
    void setAltEnds(byte ends) { altEnds = ends; }

    const char* getAltCrsCode() const { return altCrsCode; }
    void setAltCrsCode(const char* code) { strlcpy(altCrsCode, code, sizeof(altCrsCode)); }

    float getAltLat() const { return altLat; }
    void setAltLat(float lat) { altLat = lat; }

    float getAltLon() const { return altLon; }
    void setAltLon(float lon) { altLon = lon; }

    const char* getAltCallingCrsCode() const { return altCallingCrsCode; }
    void setAltCallingCrsCode(const char* code) { strlcpy(altCallingCrsCode, code, sizeof(altCallingCrsCode)); }

    const char* getAltCallingStation() const { return altCallingStation; }
    void setAltCallingStation(const char* station) { strlcpy(altCallingStation, station, sizeof(altCallingStation)); }

    const char* getAltPlatformFilter() const { return altPlatformFilter; }
    void setAltPlatformFilter(const char* filter) { strlcpy(altPlatformFilter, filter, sizeof(altPlatformFilter)); }




    const char* getLocationName() const override {
        return stationData.location;
    }

    int updateData() override;
    const char* getLastErrorMsg() const override { return lastErrorMsg; }

    void render(U8G2& display) override;
    void tick(uint32_t currentMillis) override;

    // Temporary holdovers during refactor:
    int getNumServices() const { return stationData.numServices; }
    void drawHeader(U8G2& display) const;
    void drawService(U8G2& display, int serviceIndex, int yPos) const;
    void animateTick() {}
    void dumpToSerial() const {}
};

extern NationalRailBoard nationalRailBoard;

#endif // NATIONAL_RAIL_BOARD_HPP
