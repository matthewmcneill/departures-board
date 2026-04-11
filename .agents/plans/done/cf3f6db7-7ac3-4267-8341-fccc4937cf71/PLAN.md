---
name: "OLED Sleep Configuration Refactor (v2.4)"
description: "Migrated the \\"Turn OLED Off completely in sleep\\" setting from a global system configuration to a per-board configuration within the Screensaver (`SleepingBoard`) module. Implemented a robust JSON mi..."
created: "2026-03-27"
status: "DONE"
commits: ['131c79c']
---

# Summary
Migrated the \"Turn OLED Off completely in sleep\" setting from a global system configuration to a per-board configuration within the Screensaver (`SleepingBoard`) module. Implemented a robust JSON migration path (v2.3 to v2.4) and state-tracked power management in `DisplayManager` to prevent \"stuck off\" states during board transitions. Verified the implementation through hardware flashing, serial log analysis, and Playwright E2E tests.

## Key Decisions
- **Per-Board Privacy**: Finalized the decision to move OLED power management into `BoardConfig`. This allows users to have multiple clocks with different power behaviors (e.g., a "Clock" that stays on and a "Sleep" board that turns off).
- **DisplayManager Safety**: Implemented a mandatory `setPowerSave(false)` call in `DisplayManager::showBoard()`. This acts as a centralized safety mechanism that wakes the hardware before any new board is rendered, preventing logical state leakage from the previous board.
- **State Tracking**: Added `oledPowerSaveActive` to `DisplayManager` to eliminate redundant I2C/SPI transactions to the SSD1322 controller when the power state hasn't changed.
- **v2.4 Migration Engine**: Added a targeted migration block in `ConfigManager` that extracts the legacy global `turnOffOledInSleep` and applies it to the first `MODE_CLOCK` board found, ensuring a seamless OTA upgrade for v2.3 users.
- **Web UI Interaction Fix**: Resolved a JavaScript crash in the board editor where `form.elements` mapping failed for certain board types. Optimized the DOM access to use robust string-based lookups for the `oledOff` checkbox.

**Archive**: [.agents/plans/done/cf3f6db7-7ac3-4267-8341-fccc4937cf71/](.agents/plans/done/cf3f6db7-7ac3-4267-8341-fccc4937cf71/)

## Technical Context
- [sessions.md](sessions.md)
- [task.md](task.md)
