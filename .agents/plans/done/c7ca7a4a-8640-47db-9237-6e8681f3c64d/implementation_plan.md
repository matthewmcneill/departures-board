# Implementation Plan: Generic `iButton` Interface and Touch Sensor Driver

[x] Reviewed by house-style-documentation - passed
[x] Reviewed by architectural-refactoring - passed
[x] Reviewed by embedded-systems - passed

## Goal Description
The objective is to introduce a unified hardware input abstraction (`iButton` interface) and provide a concrete implementation for the TTP223 touch sensor introduced in the upstream repository. This will allow the system to handle physical buttons, touch sensors, or other input methods interchangeably via Dependency Injection, conforming to the Single Responsibility Principle and Open/Closed Principle.

## User Review Required
> **Review Complete**: The user has selected the following mappings for input behavior:
> - **Short Tap:** Wake & Cycle (Option 1). Wakes from screensaver if asleep; switches to the next configured board if awake.
> - **Long Tap:** Manual Screensaver Toggle (Option B). Manually toggles the display into or out of forced sleep to save the OLED.

## Proposed Changes

### Input Module
We will create a new directory `modules/systemManager/input/` to encapsulate this logic.

#### [NEW] [iButton.hpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/systemManager/input/iButton.hpp)
- Define the `iButton` interface with pure virtual methods:
  - `virtual void update() = 0;` (For polling/debouncing if needed).
  - `virtual bool isPressed() = 0;`
  - `virtual bool wasShortTapped() = 0;`
  - `virtual bool wasLongTapped() = 0;`
  - `virtual int secsSinceLastTap() = 0;`

#### [NEW] [touchSensor.hpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/systemManager/input/touchSensor.hpp)
- Concrete class `TouchSensor` inheriting from `iButton`.
- Contains configuration for pins, debounce times, and long-tap threshold.
- Translates the upstream `touchSensor.h` to our house style.

#### [NEW] [touchSensor.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/systemManager/input/touchSensor.cpp)
- Implementation of the polling logic, taking the ported code from `git show upstream/main:lib/touchSensor/touchSensor.cpp` and adapting it.

### System Manager Integration

#### [MODIFY] [systemManager.hpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/systemManager/systemManager.hpp)
- Add a member `iButton* inputDevice` (dependency injected or managed internally).
- Include standard house style documentation updates if lacking.

#### [MODIFY] [systemManager.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/systemManager/systemManager.cpp)
- During `SystemManager::begin()`, instantiate the `TouchSensor` if configured.
- Call `inputDevice->update()` in `SystemManager::tick()` if it exists.

## Resource Impact Assessment
- **Flash/ROM:** Minimal impact. The `iButton` interface vtable and `TouchSensor` logic require < 2KB of flash.
- **RAM:** Minimal impact. A single instance of `TouchSensor` will consume ~32 bytes for state tracking variables.
- **Stack/Heap:** Instantiated once globally or via `new` during setup, with no dynamic allocations during runtime. No heap fragmentation risk.
- **Power:** If using polling, it requires the MCU to be awake to poll at regular intervals. However, if the system is designed to sleep, we should eventually map the `TTP223` output pin to an ESP32 ext0/ext1 wake-up source. Upstream supports waking from screensaver via touch.
- **Security:** No network impact.

## Verification Plan

### Automated Tests
- Run `pio run` to verify that the firmware compiles cleanly across all environments.

### Manual Verification
1. Flash the firmware using `/flash-test`.
2. Ensure the serial monitor shows the `SystemManager` initializing the touch sensor.
3. If hardware is available, tap the sensor (short and long) and monitor the serial output to confirm the button events are registered.
