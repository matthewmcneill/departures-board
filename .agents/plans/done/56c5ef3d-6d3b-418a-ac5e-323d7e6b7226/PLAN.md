---
name: "Unifying DataManager Dispatch & Thread Safety"
description: "Finalized the stabilization of the new `DataManager` architecture by diagnosing intermittent multi-thread hardware crashes (`LoadProhibited`) resulting from unsynchronized memory mapping between the C..."
created: "2026-03-27"
status: "DONE"
commits: ['c2d975f']
---

# Summary
Finalized the stabilization of the new `DataManager` architecture by diagnosing intermittent multi-thread hardware crashes (`LoadProhibited`) resulting from unsynchronized memory mapping between the Core 0 network fetcher and Core 1 OLED render matrices. Completed full 5-minute physical hardware validation displaying zero missed frames and perfect Priority Queue interrupts for live Web Portal E2E API tests.

## Key Decisions
- **RAII Mutex Envelopes**: Eliminated visual tearing and `LoadProhibited` segfaults by implementing explicit `lockData()` and `unlockData()` interfaces inside `iDataSource` using FreeRTOS `SemaphoreHandle_t`. The Core 0 Double-Buffer performs instant microsecond copies while Core 1 locks across its 30ms hardware rendering cycle.
- **Dual-Dispatch Unification**: Corrected a deadlock occurring within `webServer` where transient `ApiTestDataSource` objects were dropped by the queue. Consolidated the `workerTaskLoop` logic so predictive polling and sudden hardware interruptions resolve through a single, strict `executeFetch()` dispatch block.
- **Predictive Backoff Net**: Forced a global 15-second backoff in `DataManager` strictly guaranteeing that data handlers returning early on `HTTP 401/403` do not infinite-loop the core parser interval. 
- **Diagnostics Extension**: Injected real-time Free Heap mapping and internal SoC thermals out bounding across the `departuresBoard::loop` Heartbeat.

**Archive**: [.agents/plans/done/56c5ef3d-6d3b-418a-ac5e-323d7e6b7226/](.agents/plans/done/56c5ef3d-6d3b-418a-ac5e-323d7e6b7226/)

## Technical Context
- [sessions.md](sessions.md)
- [task.md](task.md)
