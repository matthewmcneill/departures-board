/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/boards/busBoard/include/busBoard.hpp
 * Description: Implementation of the IStation interface for 
 *              Bus departure boards.
 *
 * Provides:
 * - BusService: Data representation of a Bus expected arrival.
 * - BusBoard: Concrete implementation of IStation for Bus stops.
 */

#ifndef BUS_BOARD_HPP
#define BUS_BOARD_HPP

#include "../../interfaces/IStation.hpp"
#include "busDataClient.hpp"
#include "../../interfaces/messageData.h"



/**
 * @brief Data structure representing a single bus service arrival on the Transport network.
 *        Stores all the attributes needed to render the service onto the OLED display.
 */
struct BusService {
    char sTime[6];
    char destination[BUS_MAX_LOCATION];
    char routeNumber[BUS_MAX_LOCATION]; 
    char expectedTime[11];
};

/**
 * @brief Top-level data entity wrapping the localized bus stop variables and an array
 *        of the upcoming expected departures.
 */
struct BusStop {
    char location[BUS_MAX_LOCATION];
    int numServices;
    bool boardChanged;
    BusService service[BUS_MAX_SERVICES];
};

/**
 * @brief Concrete implementation of IStation for Bus departure boards.
 */
class BusBoard : public IStation {
private:
    BusStop stationData;
    char lastErrorMsg[128] = "";

    // --- Animation & Rendering State ---
    int numMessages = 0;
    int currentMessage = 0;
    int prevMessage = 0;
    int scrollStopsLength = 0;
    int prevScrollStopsLength = 0;
    int scrollStopsXpos = 0;
    int scrollStopsYpos = 0;
    bool isScrollingService = false;
    int line3Service = 0;
    int scrollServiceYpos = 0;
    int prevService = 0;
    uint32_t serviceTimer = 0;
    bool isScrollingPrimary = false;
    int scrollPrimaryYpos = 0;
    uint32_t refreshTimer = 0;
    uint32_t nextClockUpdate = 0;
    
    char line2[10][BUS_MAX_LOCATION];

    // --- Board Configuration Variables ---
    char busAtco[13] = "";
    char busName[80] = "";
    int busDestX = 0;
    char busFilter[54] = "";
    char cleanBusFilter[54] = "";
    float busLat = 0;
    float busLon = 0;
    bool enableBus = false;

public:
    BusBoard() {
        memset(&stationData, 0, sizeof(BusStop));
    }

    // --- Configuration Getters/Setters ---
    const char* getBusAtco() const { return busAtco; }
    void setBusAtco(const char* atco) { strlcpy(busAtco, atco, sizeof(busAtco)); }

    const char* getBusName() const { return busName; }
    void setBusName(const char* name) { strlcpy(busName, name, sizeof(busName)); }

    int getBusDestX() const { return busDestX; }
    void setBusDestX(int x) { busDestX = x; }

    const char* getBusFilter() const { return busFilter; }
    void setBusFilter(const char* filter) { strlcpy(busFilter, filter, sizeof(busFilter)); }

    const char* getCleanBusFilter() const { return cleanBusFilter; }
    void setCleanBusFilter(const char* filter) { strlcpy(cleanBusFilter, filter, sizeof(cleanBusFilter)); }

    float getBusLat() const { return busLat; }
    void setBusLat(float lat) { busLat = lat; }

    float getBusLon() const { return busLon; }
    void setBusLon(float lon) { busLon = lon; }

    bool getEnableBus() const { return enableBus; }
    void setEnableBus(bool enable) { enableBus = enable; }

    const char* getLocationName() const override;
    
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

extern BusBoard* busBoard;

#endif // BUS_BOARD_HPP
