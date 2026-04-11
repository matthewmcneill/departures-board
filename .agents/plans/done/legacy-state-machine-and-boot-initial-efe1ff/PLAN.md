---
name: "State Machine and Boot Initialization Refactor"
description: "Refactored the initial boot sequence in `appContext` and `DisplayManager` to cleanly transition through states (`BOOTING`, `WIFI_SETUP`, `BOARD_SETUP`, `RUNNING`) without premature board activation or..."
created: "2026-03-15"
status: "DONE"
commits: ['18f4bca']
---

# Summary
Refactored the initial boot sequence in `appContext` and `DisplayManager` to cleanly transition through states (`BOOTING`, `WIFI_SETUP`, `BOARD_SETUP`, `RUNNING`) without premature board activation or API requests under unconfigured conditions. Added a robust state management document (`AppContextStateMachine.md`).

## Key Decisions
- **`hasConfiguredBoards()` Implementation**: Modified the configuration check to evaluate `.complete` flags natively instead of blindly trusting `MAX_BOARDS` index parsing, resolving a bug where default incomplete boards bypassed the Web Setup phase.
- **Display Transition Integrity**: Fixed `appContext::tick()` to explicitly trigger `showBoard(SYS_HELP_CRS)` during the precise transition from `BOOTING` to `BOARD_SETUP`. 
- **Splash Screen Lockout**: Restructured `DisplayManager::applyConfig` to block configured boards from replacing the system `splashBoard` or `wifiWizard` until `appContext` fully transitions into the `RUNNING` state.

## Technical Context
