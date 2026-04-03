# Lazy-Loading System Boards in DisplayManager

This plan outlines the refactoring of the `DisplayManager` to transition from upfront static allocation of all system boards during boot to a lazy-loading "on-demand" pattern. This ensures that memory is only consumed for system screens that are actually displayed (e.g., WiFi Wizard, Help screens, or Error states).

## User Review Required

> [!IMPORTANT]
> **Allocation Strategy**: System boards will remain cached once loaded to ensure snappy transitions if they are needed again (e.g., returning to the Help screen). However, they will not consume any heap space until the first time they are requested.
> **Boot Latency**: The very first screen (`SYS_BOOT_SPLASH`) will be lazy-loaded immediately during `begin()`, ensuring no visible change to the boot animation flow.

## Proposed Changes

### DisplayManager Orchestrator

#### [MODIFY] [displayManager.cpp](modules/displayManager/displayManager.cpp)

- **`begin(appContext* contextPtr)`**: 
    - Remove the hardcoded block of 8 `BoardFactory::createSystemBoard` calls and their subsequent `init` checks.
    - The hardware `u8g2.begin()` will remain, followed immediately by `showBoard(getSystemBoard(SystemBoardId::SYS_BOOT_SPLASH))`.
- **`getSystemBoard(SystemBoardId id)`**:
    - Update the implementation to perform "Init-on-First-Use" for each board member (`splashBoard`, `loadingBoard`, `wizardBoard`, `helpBoard`, `messageBoard`, `firmwareUpdateBoard`, `sleepingBoard`, `diagnosticBoard`).
    - Add a check: `if (boardPointer == nullptr) { boardPointer = BoardFactory::createSystemBoard(...); if (boardPointer) boardPointer->init(context); }`.
    - Ensure shared boards (like `MessageBoard` for all Error IDs) are handled correctly so only one instance is created.

## Verification Plan

### Automated Tests
- **Build Verification**: Run `pio run -e esp32dev` to ensure no regression in the build pipeline.
- **Linting**: Verify that the new lazy-loading logic doesn't introduce any "uninitialized member" or "dereference nullptr" warnings.

### Manual Verification
- **Boot Sequence**: Verify the Gadec logo appears immediately, followed by the loading progress bar.
- **Connectivity Scenarios**:
    - Trigger `SYS_WIFI_WIZARD` by enabling AP mode (if possible in testing) or manually calling it via the web portal's test tools.
    - Trigger an error state (e.g., `SYS_ERROR_TOKEN`) to verify the `MessageBoard` lazy-loads and displays the correct content.
- **Screensaver**: Verify the clock screen appears during the scheduled sleep window.
- **Diagnostics**: Verify the diagnostic grid still functions when selected.

# Plan Wrap: Execution (Session b75d98e4-55f7-435b-a4a1-0210b6946b34)
The task is being formally concluded following the `/plan-wrap` workflow. 
1. Finalize documentation.
2. Commit changes.
3. Save plan artifacts into `.agents/plans/b75d98e4-55f7-435b-a4a1-0210b6946b34/`.
4. Release Hardware Lock.
5. Record into project log.
