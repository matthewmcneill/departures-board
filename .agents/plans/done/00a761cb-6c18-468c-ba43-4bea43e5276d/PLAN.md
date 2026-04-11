---
name: "Migration of Preprocessor Macros to Type-Safe Constants"
description: "Refactored the firmware's status reporting and scheduling systems to use type-safe `UpdateStatus` and `PriorityTier` enums. Resolved critical macro collisions between `PriorityTier` members (`LOW`, `H..."
created: "2026-04-02"
status: "DONE"
commits: ['392af1a']
---

# Summary
Refactored the firmware's status reporting and scheduling systems to use type-safe `UpdateStatus` and `PriorityTier` enums. Resolved critical macro collisions between `PriorityTier` members (`LOW`, `HIGH`) and Arduino's hardware abstraction by adopting the `PRIO_` prefix. Standardized the `iDataSource` interface to return `UpdateStatus` and updated all data sources (National Rail, TfL, Bus, RSS, Weather) to comply. Validated the changes through a clean PlatformIO build and host-based unit tests.

## Technical Context
- [sessions.md](sessions.md)
