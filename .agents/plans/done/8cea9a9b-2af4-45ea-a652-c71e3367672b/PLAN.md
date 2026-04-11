---
name: "DI Refactor & Encapsulation"
description: "Eliminated floating global singletons (`displayManager`, `currentWeather`, `server`, `manager`) in favor of context-based service access via `appContext`. Removed deprecated, UI-bleeding callbacks fro..."
created: "2026-04-02"
status: "DONE"
commits: ['2072c04']
---

# Summary
Eliminated floating global singletons (`displayManager`, `currentWeather`, `server`, `manager`) in favor of context-based service access via `appContext`. Removed deprecated, UI-bleeding callbacks from `systemManager`.

## Technical Context
- [sessions.md](sessions.md)
- [context_bridge.md](context_bridge.md)
- [task.md](task.md)
