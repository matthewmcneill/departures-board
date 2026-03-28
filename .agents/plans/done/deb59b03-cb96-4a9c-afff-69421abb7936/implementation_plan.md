# Implementation Plan: Display Orchestrator and Loading Board Fixes

**Audit Checklist:**
- [x] Reviewed by `house-style-docs` - passed (Header comments will be applied if creating new files)
- [x] Reviewed by `architectural-refactoring` - passed (`resetState` decoupling resolves state leakage)
- [x] Reviewed by `embedded-web-designer` - ASCII art provided and layout verified
- [x] Reviewed by `embedded-systems` - Resource impact: Negligible (UI coordinate shifts only)

## Goal Description
The system's display orchestration and UI rendering logic contain a few architectural and visual bugs preventing clean transitions and readable screens. We need to:
1. Fix the internal overlap within the `progressBarWidget` where the progress bar paints *over* the text string.
2. Remove the duplicated "Setting the system clock" text drawn by the `LoadingBoard`.
3. Properly anchor the Build Version string so it doesn't collide with centered text.
4. Replace the flawed `DisplayManager::resetState` method with a reliable `resumeDisplays` command, entirely decoupling it from system state assumptions.

## User Review Required

Please review the proposed ASCII layout for the fixed **Loading Board**.

**Current Broken State (Mental Model):**
```text
+------------------------------------------------+
|               Departures Board                 |
|                                                |
|                                                |
|         Se+------------------------+ck...      |  <-- Widget text & bar physically overlapping
|           |///////////////////     |           |
|                                                |
|2603182313 Setting the system clock...          |  <-- Build Time and duplicate text overlapping
+------------------------------------------------+
```

**Proposed Fixed State:**
```text
+------------------------------------------------+
|               Departures Board                 |
|                                                |
|                                                |
|          Setting the system clock...           |  <-- Widget text clean, moved up
|  [========================================]    |  <-- Bar clean, moved down
|                                                |
|v3.0                                 2603182313 |  <-- Version anchor (L) and Build Time anchor (R)
+------------------------------------------------+
```
*(Added v3.0 to the bottom left as requested!)*

---

## Proposed Changes

### Display Manager Component
#### [MODIFY] [displayManager.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/displayManager.cpp)
- Rip out `resetState()`.
- Implement `resumeDisplays()` that explicitly calls `showBoard(getDisplayBoard(activeSlotIndex))` without polling `appContext` states.

#### [MODIFY] [displayManager.hpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/displayManager.hpp)
- Rename the method signature from `resetState` to `resumeDisplays`.

### Orchestrator Component
#### [MODIFY] [appContext.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/appContext/appContext.cpp)
- Change line 125 inside `sysManager.setBootProgressCallback` to call `displayManager.resumeDisplays()`.
- Ensure all other `resetState` calls are properly migrated.

#### [MODIFY] [systemManager.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/systemManager/systemManager.cpp)
- Ensure the soft reset routine uses the updated `resumeDisplays` command.

### UI Widget Component
#### [MODIFY] [progressBarWidget.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/widgets/progressBarWidget.cpp)
- Fix the internal Y-coordinate collision. Currently, the widget is 17px tall, drawing text at `Y+11` and the bar at `Y+15` (causing them to render in the exact same physical space).
- We will anchor the text to `Y` (top of the widget bounds) and the bar to `Y + renderH - 4` (bottom of the widget bounds), granting them adequate breathing room.

#### [MODIFY] [loadingBoard.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/boards/systemBoard/loadingBoard.cpp)
- Increase the instantiation height of `pBar` from `17` to `24` pixels to allow the text and bar to safely coexist vertically.
- Remove the redundant `noticeMessage` string rendered at `Y=53`.
- Shift the `buildTime` string to right-aligned at `Y=53`.
- Draw the text `"v3.0"` explicitly left-aligned at `X=0, Y=53`.

---

## Verification Plan
### Automated Tests
- Run `pio run -e esp32dev` to ensure the orphaned `resetState` errors are resolved and the firmware compiles perfectly.

### Manual Verification
- We will instruct the hardware to reboot and monitor the live serial feed to watch the smooth transition to `RUNNING` bypass the previous freeze.
- You can physically observe the OLED screen to verify the text overlaps have disappeared.
