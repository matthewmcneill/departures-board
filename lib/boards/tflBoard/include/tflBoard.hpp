/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/boards/tflBoard/include/tflBoard.hpp
 * Description: Implementation of the IStation interface for 
 *              Transport for London (TfL) Underground boards.
 *
 * Provides:
 * - TfLService: Data representation of a London Underground departure.
 * - TfLBoard: Concrete implementation of IStation for Underground stations.
 */

#ifndef TFL_BOARD_HPP
#define TFL_BOARD_HPP

#include "../../interfaces/IStation.hpp"
#include "TfLdataClient.hpp"
#include "../../interfaces/messageData.h"

/**
 * @brief Data structure representing a single train service arrival on the London Underground.
 *        Stores all the attributes needed to render the service onto the OLED display.
 */
struct TflService {
    char destination[TFL_MAX_LOCATION];
    char lineName[TFL_MAX_LOCATION]; 
    char expectedTime[11];
    int timeToStation; 
};

/**
 * @brief Top-level data entity wrapping the localized station variables and an array
 *        of the upcoming departures from the TfL network.
 */
struct TflStation {
    char location[TFL_MAX_LOCATION];
    int numServices;
    bool boardChanged;
    TflService service[TFL_MAX_SERVICES];
};

/**
 * @brief Concrete implementation of IStation for TfL Underground departure boards.
 */
class TfLBoard : public IStation {
private:
    TflStation stationData;
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
    bool attributionScrolled = false;
    bool isScrollingPrimary = false;
    int scrollPrimaryYpos = 0;
    uint32_t refreshTimer = 0;
    uint32_t nextClockUpdate = 0;
    
    char line2[10][MAXMESSAGESIZE];

    // --- Board Configuration Variables ---
    char tflAppkey[50] = "";
    char tubeId[13] = "";
    char tubeName[80] = "";

public:
    TfLBoard() {
        memset(&stationData, 0, sizeof(TflStation));
    }

    // --- Configuration Getters/Setters ---
    const char* getTflAppkey() const { return tflAppkey; }
    void setTflAppkey(const char* key) { strlcpy(tflAppkey, key, sizeof(tflAppkey)); }

    const char* getTubeId() const { return tubeId; }
    void setTubeId(const char* id) { strlcpy(tubeId, id, sizeof(tubeId)); }

    const char* getTubeName() const { return tubeName; }
    void setTubeName(const char* name) { strlcpy(tubeName, name, sizeof(tubeName)); }

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

extern TfLBoard* tflBoard;

#endif // TFL_BOARD_HPP
