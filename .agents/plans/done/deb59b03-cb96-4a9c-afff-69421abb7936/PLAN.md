---
name: "Display Lifecycle Telemetry & Architecture Refactor"
description: "Audited the `DisplayManager` and `systemManager` coupling to ensure robust transition logic. Decoupled internal system handlers from explicit board classes by implementing the Observer Pattern in the ..."
created: "2026-03-18"
status: "DONE"
commits: []
---

# Summary
Audited the `DisplayManager` and `systemManager` coupling to ensure robust transition logic. Decoupled internal system handlers from explicit board classes by implementing the Observer Pattern in the `appContext` orchestrator. Enhanced the unified diagnostic logging pipeline to track exhaustive UI footprint events across all display modalities.

## Key Decisions
- **Orchestration Mediator**: Abstracted UI dependencies away from `systemManager` and `otaUpdater` using `std::function` callbacks. These callbacks are dynamically assigned within `appContext::begin()`.
- **Semantic UI Telemetry**: Added a required `getBoardName()` string implementation to the `iDisplayBoard` contract, forcing all 11 display boards to self-identify.
- **Contextual View Logs**: Extended `DisplayManager::showBoard(board, reason)` to accept and log the exact `reason` the viewing context changed (e.g., config save, carousel rotate, OTA interrupt).
- **Compilation Validation**: Confirmed syntactical soundness and verified that zero overhead/errors were introduced to the `esp32dev` toolchain.

**Archive**: [.agents/plans/done/deb59b03-cb96-4a9c-afff-69421abb7936/](.agents/plans/done/deb59b03-cb96-4a9c-afff-69421abb7936/)

## Technical Context
- [sessions.md](sessions.md)
- [task.md](task.md)
