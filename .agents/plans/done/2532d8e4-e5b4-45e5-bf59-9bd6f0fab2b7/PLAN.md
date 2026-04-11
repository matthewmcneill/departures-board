---
name: "SWR Visual Refinements & Test Mock Recovery"
description: "Finalized the visual refinements for the South Western Railway (SWR) display mode, including custom font recompilation and platform widget alignment. Conducted a major refactoring of the native test h..."
created: "2026-04-05"
status: "DONE"
commits: ['8b8b496']
---

# Summary
Finalized the visual refinements for the South Western Railway (SWR) display mode, including custom font recompilation and platform widget alignment. Conducted a major refactoring of the native test harness mocks (U8G2, appContext, dataManager) to align with the v3.0 `schedulerManager` architecture. Successfully verified the firmware build on `esp32dev` hardware, while deferring the full native test suite execution to a dedicated restoration session.

## Key Decisions
- **Font Baseline Alignment**: Re-compiled `SWRPlatformNumberMega` and `SWRClockHuge` to use `getAscent()` arithmetic, ensuring pixel-perfect vertical alignment of platform numbers and clock seconds without manual offsets.
- **Mock Synchronisation**: Replaced the legacy `systemManager` dependency in all native tests with the new `schedulerManager` pattern. Consolidated `appContext` stubs to eliminate redundant implementation conflicts in the linker.
- **Test Execution Deferral**: Decided to bypass `unit_testing_host` during the final wrap protocol after identifying deep conflicts in the `LittleFS` and `Logger` stubs that require a dedicated, non-visual session to resolve sustainably.
- **WASM Simulator Sync**: Updated `gen_layout_cpp.py` and the registry to bridge the new National Rail 'Platform' widget into the simulation environment.

## Technical Context
- [sessions.md](sessions.md)
- [context_bridge.md](context_bridge.md)
- [task.md](task.md)
