---
name: "Time Update & Colon Blink Fix"
description: "Diagnosed and resolved a \"stuck clock\" issue where the `clockWidget` on standard departure boards was only updating during network fetch yields. Implemented a forced `updateCurrentTime()` poll inside ..."
created: "2026-03-27"
status: "DONE"
commits: ['5abd5f3']
---

# Summary
Diagnosed and resolved a "stuck clock" issue where the `clockWidget` on standard departure boards was only updating during network fetch yields. Implemented a forced `updateCurrentTime()` poll inside the widget's `render()` method to ensure real-time accuracy during the main 60Hz display loop. Simultaneously repaired the colon blinking state machine by correctly tracking `oldColon` transitions. Verified the fix using the `flash-test` protocol on physical `esp32dev` hardware.

## Key Decisions
- **Main Loop Synchronization**: Moved the primary `updateCurrentTime()` trigger into the widget's `render()` pass. This ensures the clock remains accurate without requiring a dedicated background timer or relying on intermittent network yields.
- **State Tracking Correction**: Fixed the logical error in `renderAnimationUpdate()` where `oldColon` was never updated, which previously caused the colon to either stay static or blink erratically depending on the initial state.
- **House Style Alignment**: Audited and updated the `clockWidget.cpp` module header and documentation to strictly adhere to v3.0 project standards.

## Technical Context
- [sessions.md](sessions.md)
