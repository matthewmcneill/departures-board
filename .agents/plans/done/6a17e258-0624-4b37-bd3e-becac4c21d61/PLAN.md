---
name: "Migrated Legacy Configuration to SchedulerManager Pattern"
description: "Fully migrated legacy configuration definitions (including the removal of sleepEnabled, sleepStarts, and sleepEnds variables from system configuration) into the central SchedulerManager framework via ..."
created: "2026-04-03"
status: "DONE"
commits: ['affd35d']
---

# Summary
Fully migrated legacy configuration definitions (including the removal of sleepEnabled, sleepStarts, and sleepEnds variables from system configuration) into the central SchedulerManager framework via native runtime conversion maps targeting a generic Clock Board. Dynamically scaled UI array bounds by passing `MAX_BOARDS`, `MAX_KEYS`, and `MAX_SCHEDULE_RULES` directly to the web portal via the new `/api/config` REST endpoint. Natively updated portal `index.html` to sync time offsets and support global OTA flags. Validated through local Node.js UI tests, compilation `pio run`, and successful hardware serial boot analysis confirming data translation mappings.

## Technical Context
- [sessions.md](sessions.md)
- [context_bridge.md](context_bridge.md)
