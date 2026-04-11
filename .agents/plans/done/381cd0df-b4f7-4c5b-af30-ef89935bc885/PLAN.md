---
name: "Refactoring Logging Enums & Include Cleanup"
description: "Stripped redundant `#include` statements from `src/departuresBoard.cpp` and optimized the heartbeat telemetry loop to conditionally compile under `#if CORE_DEBUG_LEVEL >= APP_LOG_LEVEL_INFO`. Refactor..."
created: "2026-04-02"
status: "DONE"
commits: []
---

# Summary
Stripped redundant `#include` statements from `src/departuresBoard.cpp` and optimized the heartbeat telemetry loop to conditionally compile under `#if CORE_DEBUG_LEVEL >= APP_LOG_LEVEL_INFO`. Refactored `CORE_DEBUG_LEVEL` checks across the codebase to utilize a centralized declarative macro set (`APP_LOG_LEVEL_*`) inside `logger.hpp` for improved readability and strict separation from Arduino magic numbers. Validated with `pio run -e esp32dev` and `pio test`.

## Technical Context
- [sessions.md](sessions.md)
