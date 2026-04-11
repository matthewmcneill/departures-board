---
name: "Boot Progress Bar Refactor"
description: "Refactored the firmware boot sequence to provide a contiguous 0-100% linear progress bar and resolved a first-load state machine deadlock. This ensures the device correctly transitions to the `RUNNING..."
created: "2026-04-02"
status: "DONE"
commits: ['7f35aa3']
---

# Summary
Refactored the firmware boot sequence to provide a contiguous 0-100% linear progress bar and resolved a first-load state machine deadlock. This ensures the device correctly transitions to the `RUNNING` state only after the initial network data fetch is complete.

## Key Decisions
- **Contiguous Segment Mapping**: Remapped the boot phases to monotonic segments: HW/FS (0-25%), WiFi (25-50%), NTP (50-75%), and First Data (75-100%).
- **First Load Synchronization**: Integrated `networkManager.getNoDataLoaded()` into the `appContext` state machine, ensuring the loading screen persists until boards are hydrated.
- **Async Data Signal**: Updated the background `DataManager` worker to explicitly flag the first successful data fetch, solving the premature screen drop.
- **Loop-Back Removal**: Purged the legacy "loop back to 45" logic in the NTP sync phase, ensuring a unidirectional progress experience.

## Technical Context
- [sessions.md](sessions.md)
- [context_bridge.md](context_bridge.md)
- [task.md](task.md)
