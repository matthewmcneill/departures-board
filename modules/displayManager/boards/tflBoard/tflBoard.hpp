/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/boards/tflBoard/tflBoard.hpp
 * Description: Implementation of iDisplayBoard for TfL Underground boards.
 */

#ifndef TFL_BOARD_HPP
#define TFL_BOARD_HPP

class appContext;

#include "../interfaces/iDisplayBoard.hpp"
#include "tflDataSource.hpp"
#include <configManager.hpp>
#include "iTflLayout.hpp"
#include "layouts/layoutDefault.hpp"

class TfLBoard : public iDisplayBoard {
private:
    appContext* context;
    tflDataSource dataSource;
    
    // UI Layout
    iTflLayout* activeLayout;

    // Configuration
    char tflAppkey[50];
    char tubeId[13];
    char tubeName[80];
    
    // Centralized Configuration
    BoardConfig config;

    uint32_t lastUpdate;
    WeatherStatus weatherStatus;

public:
    const char* getBoardName() const override { return "DATA: TfL Board"; }
    TfLBoard(appContext* contextPtr = nullptr);
    virtual ~TfLBoard();

    // iDisplayBoard implementation
    void onActivate() override;
    void onDeactivate() override;
    void tick(uint32_t ms) override;
    void render(U8G2& display) override;
    void renderAnimationUpdate(U8G2& display, uint32_t currentMillis) override;
    void configure(const struct BoardConfig& config) override;

    // Configuration Getters/Setters
    void setTflAppkey(const char* key) { strlcpy(tflAppkey, key, sizeof(tflAppkey)); }
    void setTubeId(const char* id) { strlcpy(tubeId, id, sizeof(tubeId)); }
    void setTubeName(const char* name) { strlcpy(tubeName, name, sizeof(tubeName)); }

    const char* getTflAppkey() const { return tflAppkey; }
    const char* getTubeId() const { return tubeId; }
    const char* getTubeName() const { return tubeName; }

    int updateData() override;
    const char* getLastErrorMsg() override { return dataSource.getLastErrorMsg(); }
    WeatherStatus& getWeatherStatus() override { return weatherStatus; }
};

#endif // TFL_BOARD_HPP
