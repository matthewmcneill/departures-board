---
name: "DisplayManager Factory Refactor"
description: "Decoupled the DisplayManager from concrete board implementations by migrating to a dynamic heap-allocated Factory pattern (`BoardFactory`). Replaced static `std::variant` pools with single active `iDi..."
created: "2026-04-02"
status: "DONE"
commits: []
---

# Summary
Decoupled the DisplayManager from concrete board implementations by migrating to a dynamic heap-allocated Factory pattern (`BoardFactory`). Replaced static `std::variant` pools with single active `iDisplayBoard*` pointers to eliminate permanent static RAM consumption and prevent runtime heap fragmentation. Resolved cyclic and cascading dependency issues across the build system. Disentangled National Rail SOAP network intialization from the main UI thread by deferring WSDL discovery to the background FreeRTOS `DataManager` core. Validated stability natively via `pio run -e esp32dev`.

## Technical Context
- [sessions.md](sessions.md)
- [context_bridge.md](context_bridge.md)
- [task.md](task.md)
