/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons
 * Attribution-NonCommercial-ShareAlike 4.0 International. To view a copy of
 * this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/widgets/trainFormationWidget.hpp
 * Description: Widget for rendering train formations visually with a
 * state-machine animation loop.
 *
 * Exported Functions/Classes:
 * - trainFormationWidget: Widget for drawing dynamic train carriage layouts natively
 *   - setFormationData(): Bound the internal pointer reference
 *   - draw(): The main rendering routine
 *   - tick(): Core animation state machine loop
 */

#ifndef TRAIN_FORMATION_WIDGET_HPP
#define TRAIN_FORMATION_WIDGET_HPP

#include "../boards/nationalRailBoard/iNationalRailDataProvider.hpp"
#include "iGfxWidget.hpp"
#include <Arduino.h>
#include <U8g2lib.h>

extern const uint8_t FormationIcons7[];

/**
 * @brief Widget for rendering train formations visually.
 *        Maintains a state-machine animation loop mapping carriage data.
 */
class trainFormationWidget : public iGfxWidget {
private:
  NrCoachFormation *formations = nullptr;
  uint8_t numCoaches = 0;

  enum class Mode { MODE_FACILITIES, MODE_NUMBER, MODE_LOADING };
  Mode currentMode = Mode::MODE_NUMBER;
  uint32_t lastAnimationMs = 0;
  uint32_t currentTickMs = 0;
  uint32_t intervalMs = 3000;
  int scrollOffset = 0;

  const int MIN_COACH_WIDTH = 15;
  const int MAX_COACH_WIDTH = 20;

public:
  /**
   * @brief Constructor for the widget.
   * @param _x X coordinate on the OLED.
   * @param _y Y coordinate on the OLED.
   * @param _w Width in pixels (-1 for dynamic).
   * @param _h Height in pixels (-1 for dynamic).
   */
  trainFormationWidget(int _x, int _y, int _w = -1, int _h = -1);

  /**
   * @brief Virtual destructor.
   */
  ~trainFormationWidget() override = default;

  /**
   * @brief Injects new mock or real formation data pointers.
   * @param data Pointer to the first formation logic chunk.
   * @param count Number of coaches in this train formation.
   */
  void setFormationData(NrCoachFormation *data, uint8_t count);

  /**
   * @brief Periodic logic update for animation and mode.
   * @param ms Current system time in milliseconds.
   */
  void tick(uint32_t ms) override;

  /**
   * @brief Renders the widget to the display buffer.
   * @param display Reference to the global U8g2 instance.
   */
  void render(U8G2 &display) override;
};

#endif // TRAIN_FORMATION_WIDGET_HPP
