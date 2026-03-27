/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/boards/tflBoard/tflBoard.hpp
 * Description: Controller for TfL Underground departure boards. Manages 
 *              interaction with the TfL Unified API for arrival predictions 
 *              at a specific station.
 *
 * Exported Functions/Classes:
 * - TfLBoard: Core controller class for Underground displays.
 *   - onActivate() / onDeactivate(): Lifecycle hooks for display transitions.
 *   - tick(): Logic update for timing and scrollers.
 *   - render(): Full frame drawing.
 *   - renderAnimationUpdate(): Targeted redraw for animation quality.
 *   - updateData(): Initiates JSON fetch from TfL API.
 *   - configure(): Applies BoardConfig settings to local state.
 *   - getLastErrorMsg(): Accessor for data source error strings.
 *   - getWeatherStatus(): Accessor for shared weather state.
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
    /**
     * @brief Construct a new TfL Board instance.
     * @param contextPtr Pointer to the global application context.
     */
    TfLBoard(appContext* contextPtr = nullptr);

    /**
     * @brief Cleanup allocated resources and layouts.
     */
    virtual ~TfLBoard();

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
     * @brief Set the TfL Unified API application key.
     * @param key C-string key.
     */
    void setTflAppkey(const char* key) { strlcpy(tflAppkey, key, sizeof(tflAppkey)); }

    /**
     * @brief Set the Naptan ID for the Underground station.
     * @param id Station ID string.
     */
    void setTubeId(const char* id) { strlcpy(tubeId, id, sizeof(tubeId)); }

    /**
     * @brief Set the human-readable station name.
     * @param name Name string.
     */
    void setTubeName(const char* name) { strlcpy(tubeName, name, sizeof(tubeName)); }

    /** @return Current API key */
    const char* getTflAppkey() const { return tflAppkey; }
    /** @return Target Naptan ID */
    const char* getTubeId() const { return tubeId; }
    /** @return Station name */
    const char* getTubeName() const { return tubeName; }

    /**
     * @brief Trigger an asynchronous data refresh from TfL APIs.
     * @return UPD_SUCCESS, UPD_PENDING, or error code.
     */
    int updateData() override;

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

#endif // TFL_BOARD_HPP
