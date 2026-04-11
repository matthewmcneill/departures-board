---
name: "Board Background Sync"
description: "Successfully migrated the legacy blocking `WebServer` logic to completely asynchronous handling using `ESPAsyncWebServer`, decoupling the core API pulling logic and unblocking ESP32 thread availabilit..."
created: "2026-03-22"
status: "DONE"
commits: ['64db430']
---

# Summary
Successfully migrated the legacy blocking `WebServer` logic to completely asynchronous handling using `ESPAsyncWebServer`, decoupling the core API pulling logic and unblocking ESP32 thread availability. Implemented a robust 'Round-Robin' API polling distribution matrix natively into the central `systemManager` object hierarchy, resolving long-standing issues concerning background connectivity caching and data synchronization throttling for National Rail, TfL, and Bus displays.

## Key Decisions
- **Async Diagnostic Payloads**: Refactored chunking memory payload buffers `_tempObject` to dynamically protect explicit verification queries targeting the API Key components.
- **Geographic Data Validation**: Re-integrated the upstream `/stationpicker` handler gracefully to reconstruct missing geospatial tracking coordinates for NRE stations.
- **API Distribution Array**: Implemented the mathematical `(config.apiRefreshRate / config.boardCount)` interval loop to gracefully execute non-blocking, parallel background board synchronization queues perfectly avoiding upstream developer rate limits.
- **Fast-Fill Initialization**: Instantiated a priority 'Fast-Fill' interval state (`2000ms`) prioritizing aggressive queueing of offline hardware configurations (`lastUpdateStatus == -1`) universally at boot.
- **Dynamic UX Feedback**: Overrode static drawing elements throughout transport layout structures with real-time `Loading data...` status overlays when boards accurately resolve back to `UPD_PENDING` (`9`) execution modes.

**Archive**: [.agents/plans/done/470d6a16-dd63-42c0-bc27-62a58dac6dd2/](.agents/plans/done/470d6a16-dd63-42c0-bc27-62a58dac6dd2/)

## Technical Context
- [sessions.md](sessions.md)
- [task.md](task.md)
