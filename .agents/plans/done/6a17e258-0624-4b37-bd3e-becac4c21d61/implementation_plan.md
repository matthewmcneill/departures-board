# Schema Migration v2.5

[x] Reviewed by `house-style-documentation` - passed (naming conventions and headers applied)
[x] Reviewed by `architectural-refactoring` - passed (verified global state containment for offset)
[x] Reviewed by `embedded-web-designer` - passed (ASCII art mockups supplied)
[x] Reviewed by `embedded-systems` - passed (resource impact assessment added)

## Goal
The objective of this plan is to execute a rigorous C++ json schema migration that eliminates legacy state variables, hands over scheduling responsibility natively to `SchedulerManager`, creates the missing Web Portal inputs, and synchronizes compilation memory bounds with the browser UI.

## User Review Required
> [!IMPORTANT]
> Please review the Resource Impact Assessment and the ASCII mockups for the new UI fields to confirm they match your expectations.

## Resource Impact Assessment
- **Memory (RAM/Heap/Stack)**: Increasing `MAX_SCHEDULE_RULES` to `24` adds exactly 180 bytes to `.bss` (9 rules * 20 bytes/rule). This is negligible. JSON deserialization overhead is neutralized by the removal of the 4 legacy `sleep` variables per board. Stack footprint is unchanged.
- **ROM/Flash**: Deleting the sprawling legacy JS migration code in `index.html` shrinks the Web UI binary payload.
- **Power**: Shifting sleep management strictly to `SchedulerManager` and chaining `oledOff` to `sleepingBoard::onActivate` ensures OLED screen hardware is hard-powered down during quiet hours.
- **Security**: No cryptographic or attack surface impacts. Network boundaries remain identical.

## Proposed Changes

### System Limits Synchronization (Dynamic Extensibility)
We will export backend memory limits to the frontend so `MAX_BOARDS` and other limits can be natively increased via `-D` compiler flags in `platformio.ini` without adjusting javascript files. **Additionally, the default limit for Schedule Rules will be increased.**

#### [MODIFY] [departuresBoard.hpp](file:///Users/mcneillm/Documents/Projects/departures-board/src/departuresBoard.hpp)
- Change `#define MAX_SCHEDULE_RULES 15` to **`24`**, expanding the out-of-the-box routing capabilities safely within ESP32 RAM limits.

#### [MODIFY] [webHandlerManager.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/webServer/webHandlerManager.cpp)
- **Export Limits:** In `handleGetConfig`, append a `"limits"` object to the root JSON payload containing `MAX_BOARDS`, `MAX_KEYS`, and `MAX_SCHEDULE_RULES` parsed directly from C++ compilation macros.

#### [MODIFY] [index.html](file:///Users/mcneillm/Documents/Projects/departures-board/web/index.html)
- **Dynamic Limits:** Change the hardcoded UI caps (`const MAX_BOARDS = 6;`, `const MAX_KEYS = 5;`, `const MAX_SCHEDULE_RULES = 15;`) to mutable `let` declarations.
- Inside `app.loadSettings`, intercept the `config.limits` payload and update the javascript boundary variables instantly, empowering users with custom hardware to unlock infinite slots.

---

### Configuration Backend (C++ Schema Migration)
We will increase the JSON version tracked within `configManager.cpp` to **v2.5**.

#### [MODIFY] [configManager.hpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/configManager/configManager.hpp)
- Remove `sleepEnabled`, `sleepStarts`, `sleepEnds`, `showClockInSleep` from the `Config` struct.
- Remove `defaultBoardIndex` from the `Config` struct.
- Update `configVersion` default float from `2.4f` to `2.5f`.
*(Note: `timeOffset` will safely remain inside `BoardConfig` instead of moving to global scope).*

#### [MODIFY] [configManager.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/configManager/configManager.cpp)
- **v2.5 Migration Logic in `loadConfig()`:**
  - **Sleep Migration Flow:** If `settings["version"]` < 2.5, detect legacy `sleep` attributes. If found, we will append a `MODE_CLOCK` entry natively into the `config.boards` array, and push a new entry into `config.schedules` (mapping `start` and `endH/M` to the new clock board index).
  - **Mode Migration Flow:** Detect legacy `mode` (Index). If `mode` != 0, structurally swap `config.boards[mode]` into `config.boards[0]` so the legacy default gracefully becomes the top item sequence.
- **Serialization Cleanup:**
  - Remove serialization lines for legacy `sleep`, `sleepStarts`, `sleepEnds`, `clock`, and `mode` inside `save()` and `writeDefaultConfig()`.

---

### Display Architecture (C++ Controller)

Because we are removing legacy sleep features, we need to defer all time-gating fully to `SchedulerManager`.

#### [MODIFY] [displayManager.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/displayManager.cpp)
- Clean up `tick()` by deleting the `isSnoozing()` logic which aggressively swapped the `currentBoard` to `SYS_SLEEP_CLOCK`.
- Connect OLED hardware-power hook into `showBoard` (or natively in `sleepingBoard::onActivate()`) so power is properly shut off when a scheduler automatically rotates to a `MODE_CLOCK` board configured with `oledOff`.

---

### Frontend Web UI

#### [MODIFY] [index.html](file:///Users/mcneillm/Documents/Projects/departures-board/web/index.html)
- **Delete Javascript Migration Tracker:** Delete `migrateLegacySleep()` from the `app.loadSettings` lifecycle. C++ will solely handle this JSON migration upon load.
- **Enable Weather for Tube Boards:** Update `handleBoardTypeChange` so that when `t === 1` (`MODE_TUBE`), the UI correctly displays the Lat/Lon coordinate fields and the Weather Overlay checkbox instead of hiding them.
- **Add Daily Check & Expose Time Offset:** Add new UI inputs.

**Mockup 1: System Tab (Daily Update)**  
```text
+---------------------------------------------------+
|  System Settings                                  |
|                                                   |
|  [ ] Enable Firmware Updates                      |
|  [x] Enable Daily Automatic Updates   <-- NEW     |
|                                                   |
+---------------------------------------------------+
```

**Mockup 2: Board Editor -> Advanced Options (Rail Only)**  
```text
+---------------------------------------------------+
|  Edit Board                                       |
|  Transport Mode: [ National Rail  v ]             |
|                                                   |
|  v Advanced Options                               |
|    [x] Display Weather Overlay                    |
|    Time Offset (mins): [ 120 ]        <-- NEW     |
+---------------------------------------------------+
```

## Verification Plan

### Automated Tests
- C++ Compilation checks via PlatformIO.
- Ensure ESP32 Firmware flash boots cleanly and validates `2.5f` JSON migration successfully transforms old files.

### Manual Verification
- **Web UI**: Access the UI and verify that 24 schedules can be added without the UI throwing an alert.
- **Tube Boards**: Switch a board to Tube, check that Lat/Lon and Weather inputs are no longer hidden.
- **Rail Boards**: Switch a board to Rail, open Advanced Options, verify Time Offset is visible. Switch to Bus, verify it vanishes.
- **Sleep Roll**: Set a Schedule Rule for the current time to engage a Clock Board with `oledOff` true. Validated screen dynamically cuts physical power.
