---
name: "Memory Stability Refactoring (String Removal)"
description: ""
created: "2026-04-02"
status: "DONE"
commits: ["aa2a03c"]
---

# Summary
Successfully refactored the National Rail (Darwin) data provider to eliminate legacy `goto` cleanup patterns. The module now utilizes a modern RAII `ScrubGuard` to ensure deterministic heap scrubbing of transient XML parser strings. This migration resolved persistent "jump to label" compilation errors and improved memory stability on the Nano ESP32 hardware, maintaining a consistent 193KB free heap during high-load parsing cycles.


## Technical Context
- [sessions.md](sessions.md)
- [context_bridge.md](context_bridge.md)
