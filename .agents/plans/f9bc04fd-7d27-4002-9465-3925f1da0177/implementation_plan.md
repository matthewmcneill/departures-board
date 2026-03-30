# Implementation Plan: Simulator Logic Injection Wiring

This plan outlines the steps to connect the simulator's **Logic Injection** panel (WiFi, Weather, OTA) to the underlying C++ widgets, enabling full behavioral verification of the firmware's UI logic.

## User Review Required

> [!IMPORTANT]
> This plan modifies core test mocks (`WiFi.h`, `systemManager`) to become stateful. This is necessary for the simulator but must be done carefully to ensure it doesn't break existing native host unit tests.

## Proposed Changes

### [Simulator Engine & Mocks]

#### [MODIFY] [WiFi.h](file:///Users/mcneillm/Documents/Projects/departures-board/test/mocks/WiFi.h)
- Replace the hardcoded `status()` return value with a member variable.
- Add `setStatus(wl_status_t)` and `setRSSI(int)` methods to the `WiFiClass` mock.

#### [MODIFY] [appContext.hpp](file:///Users/mcneillm/Documents/Projects/departures-board/tools/layoutsim/src/appContext.hpp)
- Implement `systemManager` setter methods.
- Wire these setters to update the global `WiFi` mock and the active board's `WeatherStatus`.

#### [MODIFY] [main.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/tools/layoutsim/src/main.cpp)
- Enhance the `syncData()` loop to ensure that when logic injection occurs, all relevant widgets are immediately refreshed.
- Map the "OTA Progress" value to a specific `progressBarWidget` if it exists in the active layout.

### [Simulator Web UI]

#### [MODIFY] [index.html](file:///Users/mcneillm/Documents/Projects/departures-board/tools/layoutsim/web/index.html)
- Add an "RSSI/Signal Strength" slider to the Logic Injection panel (currently it only has a "WiFi Connected" toggle).
- Add a "Weather Condition" selector (Clear, Rain, Snow) to exercise the different icons in `weatherWidget`.

## Verification Plan

### Automated Tests
- Build WASM: `python3 tools/layoutsim/scripts/build_wasm.py`
- Run the dev server and use the browser subagent to:
    - Toggle WiFi and verify the `wifiStatusWidget` icon blinks and changes.
    - Change temperature/icon and verify the `weatherWidget` updates.
    - Slider the OTA progress and verify a progress bar appears and fills.

### Manual Verification
- Visually confirm that the "X" (disconnected) icon appears on the OLED when "WiFi Connected" is unchecked.
- Verify that the BBC news ticker continues to scroll while OTA is in progress.
