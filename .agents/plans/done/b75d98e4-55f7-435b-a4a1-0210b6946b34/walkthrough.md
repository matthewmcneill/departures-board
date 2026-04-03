# Walkthrough - DisplayManager Lazy-Loading Refactor

We have successfully transitioned the `DisplayManager` from upfront static allocation of all system boards to a reactive, lazy-loading pattern. This optimization ensures that heap memory is only allocated for system screens when they are actually called for display.

## Changes Made

### DisplayManager Optimization
- **`DisplayManager::begin()`**: Removed the block of 8 `BoardFactory` allocation calls. The system now boots with zero initial allocated system boards, saving immediate heap space.
- **`DisplayManager::getSystemBoard(id)`**: This is now the "Single Source of Truth" for system board access. It checks if the requested board is unallocated (`nullptr`) and dynamically creates and initializes it on the first request.
- **Unified Logic**: All direct pointer accesses to system boards (e.g., in `setDiagMode` or context fallbacks) have been replaced with `getSystemBoard(id)` calls to ensure they are safely hydrated before use.

### Architecture Improvements
- **Resource Conservation**: Boards like the `WizardBoard` (WiFi setup) or `HelpBoard` now occupy 0 bytes of RAM on a device that is already correctly configured and never needs to show those screens.
- **Persistence**: Once a board is loaded, it remains cached in its respective pointer to maintain instant responsiveness for future transitions.

## Verification Results

### Automated Tests
- **Clean Build**: Verified with `pio run -e esp32dev`. The build is successful and maintains its 19.3% RAM usage footprint (baseline static RAM).

### Manual (Code) Audit
- **Boot Safety**: The `begin()` method explicitly calls `getSystemBoard(SYS_BOOT_SPLASH)` at the very end, ensuring the splash screen is the first thing allocated and displayed, preserving the original boot experience.
- **Thread Safety**: Access patterns remain on the primary UI thread, ensuring no race conditions during the dynamic `new` allocations.

> [!NOTE]
> Even though we are using dynamic allocation (`new`), because these system boards are singletons that are only created once and then cached, we still maintain the project's goal of "Zero Steady-State Fragmentation."
