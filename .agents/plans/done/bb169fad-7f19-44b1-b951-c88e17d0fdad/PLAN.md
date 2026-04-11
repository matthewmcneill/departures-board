---
name: "Boot Sequence Optimization & Fetch Guarding"
description: "Optimized the ESP32 firmware boot sequence to reduce perceived latency and eliminate redundant network traffic. Implemented strict state-driven guards in `DataManager` and source clients (`weatherClie..."
created: "2026-03-28"
status: "DONE"
commits: ['e21917d']
---

# Summary
Optimized the ESP32 firmware boot sequence to reduce perceived latency and eliminate redundant network traffic. Implemented strict state-driven guards in `DataManager` and source clients (`weatherClient`, `rssClient`) to prevent premature API fetches during the `BOOTING` phase. Resolved a significant regression in the startup animation and UI responsiveness caused by scheduler interference during the NTP sync block.

## Key Decisions
- **Stable-State Initialization**: Deferred non-critical configuration notifications (like `resumeDisplays`) until the system reaches the `RUNNING` state, ensuring the OLED stays locked to the `LoadingBoard` during critical connect cycles.
- **Fetch Site Guarding**: Implemented "silent guards" directly inside `executeFetch()` for all data sources. This acts as a final firewall against unconfigured or early-state network requests, regardless of who invokes the update.
- **Scheduler Suspension**: Wrap the `DisplayManager` adaptive schedule and carousel rotation in an `AppState::RUNNING` guard, preventing the display from "flickering" or swapping boards while system services are still initializing.
- **Smooth Animation Interpolation**: Restored fluid progress bar movement by explicitly providing a `500ms` animation duration to `setProgress()` calls during the blocked clock synchronization phase.

**Archive**: [.agents/plans/done/bb169fad-7f19-44b1-b951-c88e17d0fdad/](.agents/plans/done/bb169fad-7f19-44b1-b951-c88e17d0fdad/)

## Technical Context
- [sessions.md](sessions.md)
- [context_bridge.md](context_bridge.md)
- [task.md](task.md)
