---
name: "Centralized Data Worker Queue Refactor"
description: "Addressed heap fragmentation and OOM panics in `WiFiClientSecure` caused by concurrent multi-board data polling. Implemented a centralized `dataWorker` queue on Core 0 to serialize external API reques..."
created: "2026-03-19"
status: "DONE"
commits: ['8a58e10']
---

# Summary
Addressed heap fragmentation and OOM panics in `WiFiClientSecure` caused by concurrent multi-board data polling. Implemented a centralized `dataWorker` queue on Core 0 to serialize external API requests across all transport modules (`weather` and `nationalRail/tfl/bus`).

## Key Decisions
- **Serialized Worker Task**: Created a singleton FreeRTOS queue (`dataWorker.hpp`) to enforce one-at-a-time network TLS handshakes, drastically reducing peak memory usage without sacrificing UI rendering thread availability.
- **iDataSource Contract Upgrade**: Promoted `executeFetch()` to the public `iDataSource` abstraction, stripping autonomous `xTaskCreatePinnedToCore` logic out of individual client modules.
- **Deduplication Blocking**: Standardized `UPD_PENDING` tracking in local source modules to prevent duplicate task queued submissions when a network request is already processing.

**Archive**: [.agents/plans/done/206c4ba2-972f-4741-96a6-a4b918e935de/](.agents/plans/done/206c4ba2-972f-4741-96a6-a4b918e935de/)

## Technical Context
- [sessions.md](sessions.md)
- [task.md](task.md)
