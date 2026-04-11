---
name: "Safe Reconfiguration & Boot Stability"
description: "Diagnosed and resolved a fatal `LoadProhibited` race condition occurring during configuration reloads. Implemented a deferred synchronization pattern in `ConfigManager` and `appContext` to ensure layo..."
created: "2026-03-23"
status: "DONE"
commits: []
---

# Summary
Diagnosed and resolved a fatal `LoadProhibited` race condition occurring during configuration reloads. Implemented a deferred synchronization pattern in `ConfigManager` and `appContext` to ensure layout memory is only manipulated by the primary rendering core. Optimized National Rail initialization by implementing a standalone `/darwin_wsdl_cache.json` system that bypasses legacy WSDL fetches while maintaining dynamic discovery and self-healing.

## Key Decisions
- **Deferred Reconfiguration**: Introduced an atomic `reloadPending` flag to decouple the `AsyncWebServer` network thread from the `DisplayManager` layout lifecycle, preventing memory access violations during I2C draw cycles.
- **Continuous status Polling**: Fixed a "Loading..." UI deadlock by allowing the round-robin controller to continuously poll `updateData()` for pending boards, ensuring display widgets can read completion states from the background `DataWorker`.
- **Standalone WSDL Cache**: Migrated the National Rail SOAP discovery results from `config.json` to a dedicated `/darwin_wsdl_cache.json` file. This preserves configuration purity while achieving millisecond boot times for the transport data source.
- **Dynamic Discovery & Self-Healing**: Configured the system to perform a live WSDL fetch only if the cache is missing or if the current endpoint returns a 404/5xx error, ensuring long-term API adaptability without daily performance penalties.
- **Sequential Test Routing**: Enforced that all web portal API validation tests must be enqueued through the `DataWorker` to prevent concurrent TLS allocation crashes.

## Technical Context
- [sessions.md](sessions.md)
