---
name: "RAII and Memory Management Refactor"
description: "Systematically migrated the core firmware from manual heap management (`new`/`delete`) to C++14 RAII paradigms using `std::unique_ptr` and `std::make_unique`. Refactored `systemManager`, `WebServerMan..."
created: "2026-04-02"
status: "DONE"
commits: []
---

# Summary
Systematically migrated the core firmware from manual heap management (`new`/`delete`) to C++14 RAII paradigms using `std::unique_ptr` and `std::make_unique`. Refactored `systemManager`, `WebServerManager`, `WifiManager`, and all transport data sources. Resolved critical "incomplete type" build errors by moving constructors, destructors, and setters to `.cpp` files. Enforced project-wide documentation standards for `std::move()` ownership transfers. Verified via `pio run -e esp32dev`.

## Technical Context
- [sessions.md](sessions.md)
- [context_bridge.md](context_bridge.md)
- [task.md](task.md)
