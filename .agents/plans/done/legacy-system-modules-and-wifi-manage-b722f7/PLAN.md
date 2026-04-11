---
name: "System Modules and WiFi Manager Integration"
description: "Executed an architectural refactoring to strictly enforce dependency injection and eliminate global singletons. Moved `appContext` to its own namespace (`modules/appContext`) to resolve LDF scoping is..."
created: "2026-03-15"
status: "DONE"
commits: ['0c8551e']
---

# Summary
Executed an architectural refactoring to strictly enforce dependency injection and eliminate global singletons. Moved `appContext` to its own namespace (`modules/appContext`) to resolve LDF scoping issues and migrated `WiFiConfig` to `modules/wifiManager`. `include/buildOptions.h` was consolidated into `src/departuresBoard.hpp` as the primary configuration header.

## Key Decisions
- **`appContext` Module**: Isolated `appContext` to fix PlatformIO Library Dependency Finder (LDF) cyclic dependency parsing when generic libraries included `.hpp` files from the consumer `src/` directory.
- **Global `wifiManager` Removal**: Refactored `wifiManager` into a scoped member of `appContext`, transitioning all legacy handlers to retrieve it via `appContext.getWifiManager()`. 
- **House Style Compliance**: Rewrote all refactored file headers to strictly adhere to the `camelCase.hpp` and Doxygen standard.

## Technical Context
