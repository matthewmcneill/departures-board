# Implementation Plan: Scheduling System & Manual Overrides

[x] Reviewed by `embedded-systems`: Resource impact assessed. Minimal static RAM overhead confirmed.
[x] Reviewed by `embedded-web-designer`: Strict Vanilla JS, <20KB UI implementation proposed for the 24h schedule view.
[x] Reviewed by `oo-expert`: Architectural decoupling ensured via a dedicated `schedulerManager`.
[x] Reviewed by `house-style-docs`: Formatting, alerts, Section Headers, and `camelCase` conventions applied.

## Goal Description
Implement a robust, time-based scheduling system that orchestrates which display boards are shown at specific times of the day. A new Web UI will allow 24-hour visualization. 

**Simplified Mapping Model**:
- One time block strictly defines the display of **One Board**.
- The same board can be scheduled across multiple distinct time blocks.
- **Implicit Carousel**: If two or more time blocks overlap chronologically, the system implicitly rotates between those scheduled boards using the existing carousel logic.

Physical interaction with the hardware suspends the schedule in favor of a full carousel rotation (Manual Override), automatically reverting after a configured timeout. Intelligent Auto-Power Off is handled implicitly by scheduling the `screensaverBoard` during off-hours.

## User Review Required
> [!IMPORTANT]
> The design has incorporated your feedback regarding the 1-to-1 mapping. Please review the updated `ScheduleRule` struct below to confirm the schema meets your expectations before we proceed to coding.

## Resource Impact Assessment
> [!TIP]
> - **Memory (RAM/Flash)**: Storing a fixed array of e.g. 15 `ScheduleRule` structs in `Config` increases the global state footprint by roughly ~150 bytes. This is negligible and avoids heap fragmentation entirely.
> - **Performance**: Evaluating the schedule runs natively against the local integer POSIX time; there are zero networking penalties.
> - **Web UI Footprint**: Visualizing a 24-hour grid will be accomplished using native flexbox layouts and HTML5 `<input type="time">`. No external calendar libraries will be used, keeping the binary payload impact at < 5KB.

## Proposed Changes

### Core Logic & State Management

#### [NEW] [schedulerManager.hpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/schedulerManager/schedulerManager.hpp)
#### [NEW] [schedulerManager.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/schedulerManager/schedulerManager.cpp)
- **Responsibility**: A dedicated manager to encapsulate time-based evaluation, ensuring Single Responsibility. 
- **State**: Maintains `bool isManualOverrideActive` and `unsigned long overrideTimestamp`.
- **Primary Methods**: 
  - `triggerManualOverride()`: Sets `isManualOverrideActive = true` and resets `overrideTimestamp = millis()` on **every single invocation**. This ensures the timeout counter restarts from zero every time the user presses the button.
  - `std::vector<int> getActiveBoards()`: Evaluates the current local time against `config.schedules`. 
    - If a manual override is active (and `millis() - overrideTimestamp < timeout`), it returns *all* structurally completely configured boards. If the timeout has expired, it resets the override state.
    - If NOT active, it searches for all rules where `CurrentTime >= Rule.Start` AND `CurrentTime <= Rule.End`.
    - **Robustness Check**: If a rule maps to a `boardIndex` that is empty or marked as `complete = false` in the ConfigManager, that rule is safely ignored to prevent rendering blank hardware.
    - It returns an array of the newly valid `boardIndex` integers from all overlapping valid rules.
    - **Empty Schedule Fallback**: If zero valid rules are found (e.g., the schedule is entirely empty, or no time blocks match the current hour), it implicitly returns *all* configured boards to ensure the device continues to carousel normally rather than remaining blank.

#### [MODIFY] [configManager.hpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/configManager/configManager.hpp)
- Add a new `ScheduleRule` struct (Simplified 1-to-1 mapping):
  ```cpp
  struct ScheduleRule {
      int startHour = 0;
      int startMinute = 0;
      int endHour = 23;
      int endMinute = 59;
      int boardIndex = -1; // -1 indicates an empty/inactive rule slot
  };
  ```
- Add `ScheduleRule schedules[MAX_SCHEDULE_RULES];` (e.g., max 15) to the `Config` struct.
- Add `int manualOverrideTimeoutSecs = 60;` to the `Config` struct.
- Add `int carouselIntervalSecs = 120;` to the `Config` struct (Default to 2 minutes).

#### [MODIFY] [displayManager.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/displayManager.cpp)
- Refactor the carousel rotation logic (`cycleNext()`).
- Instead of unconditionally rotating `activeBoardIndex = (activeBoardIndex + 1) % config.boardCount`, it will query `appContext::getSchedulerManager()->getActiveBoards()` and rotate solely through that valid subset.
- Update the rotation pacing logic to use `config.carouselIntervalSecs` instead of any hardcoded intervals, allowing user configuration of the carousel rotation speed.

#### [MODIFY] [systemManager.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/systemManager/systemManager.cpp)
- Hook into the existing `buttonHandler` (or physical GPIO interrupt system). 
- When the button is pressed to skip a board, invoke `appContext::getSchedulerManager()->triggerManualOverride()`.

### Web Portal Integration

#### [MODIFY] [index.html](file:///Users/mcneillm/Documents/Projects/departures-board/web/index.html)
- Transform the `#tab-schedule` placeholder into a functional UI designed for Mobile-First usage (Pico CSS).
- **Logic**: 
  - Expand the JS `saveAll()` payload to serialize the array of `ScheduleRule` objects back to the C++ backend.
  - **Orphan Handling**: If a rule targets a `boardIndex` that is now empty, render the rule with a red warning badge ("Target Missing").

#### UI Wireframe (Mobile View - List Paradigm)
```text
+------------------------------------------------+
|  [Test Devices]                 [Save & Apply] |
+------------------------------------------------+
|  General Settings                              |
|  [ Override Timeout: 60s                  ▼ ]  |
|  [ Carousel Interval: 120s                ▼ ]  |
|                                                |
|  [ 24-Hour Timeline ]                  [+ ADD] |
| ---------------------------------------------- |
|  07:00 - 09:00                                 |
|  [Rail] Home Commute             [EDIT] [DEL]  |
| ---------------------------------------------- |
|  09:00 - 18:00                                 |
|  [Clock] Office Desk Default     [EDIT] [DEL]  |
+------------------------------------------------+
```

### Option B: Visual Calendar Grid (Interactive Review Request)
> [!WARNING]
> You requested an analysis of implementing a "Day Schedule" visual calendar (similar to Google Calendar) instead of the list paradigm.

**Impact Analysis & Trade-offs:**

1.  **Payload & Complexity (Medium Impact)**: The list approach requires very little logic. A calendar grid requires Vanilla JavaScript to map minutes to pixel heights (`top` and `height` CSS properties) and complex logic to calculate the horizontal squashing of blocks when they overlap chronologically. 
2.  **Mobile Usability (High Impact)**: A 24-hour vertical grid requires extensive vertical scrolling. On a mobile phone (320px-480px wide), overlapping blocks will squash horizontally, making Board names harder to read.
3.  **Feasibility**: It **IS feasible** to build this purely with Vanilla JS and CSS Grid without breaking our 20KB budget, provided we make the grid "Read-Only" for touch interactions. (i.e. to add/edit, you use a standard modal dialog, you don't drag-and-drop the blocks).

#### UI Wireframe (Visual Calendar Paradigm)
```text
+------------------------------------------------+
|  Schedule & Auto-Power                 [+ ADD] |
+------------------------------------------------+
|      |                                         |
|  06  |-----------------------------------------|
|      |                                         |
|  07  |-----------------------------------------|
|      | +-------------------------------------+ |
|  08  |-| 07:00 - 09:00                       |-|
|      | | [Rail] Home Commute                 | |
|  09  |-+-------------------------------------+-|
|      | +------------------+                    |
|  10  |-| 09:00 - 12:00    |                    |-|
|      | | [Clock] Default  | +----------------+ |
|  11  |-|                  |-| 10:30 - 18:00  |-|
|      | |                  | | [Tube] M&J Walk| |
|  12  |-+------------------+ |                |-|
|      |                      |                | |
|  13  |----------------------|                |-|
|      |                      |                | |
|  14  |----------------------|                |-|
|      |                      +----------------+ |
|  15  |-----------------------------------------|
|      |                                         |
+------------------------------------------------+
```

> [!NOTE]
> **Decision Log**: The user has reviewed both options and explicitly selected **Option A: List Paradigm** for the initial implementation due to simplicity and mobile usability. **Option B** is retained here purely for future reference and is currently out of scope.

## Verification Plan

### Automated Tests
- Build and execute `npx playwright test` locally within `test/web/`.
- Verify the Web UI renders the Schedule Tab correctly, ensures no JS crashes occur when loading mock data, and specifically tests the serialization of the new 1-to-1 payload structure.

### Manual Verification
- Compile and flash to the ESP32 using `pio run -t upload`.
- **Test 1 (Overlapping Carousel)**: Create Rule A (09:00-11:00 -> Board 0) and Rule B (10:00-12:00 -> Board 1). Set device time to 10:30. Verify the carousel implicitly kicks in and rotates between Board 0 and 1.
- **Test 2 (Button Override)**: Press the hardware button. Verify the system breaks out of the scheduled constraint instantly, rotates through all boards, and automatically returns to the scheduled subset after `manualOverrideTimeoutSecs` expires.
