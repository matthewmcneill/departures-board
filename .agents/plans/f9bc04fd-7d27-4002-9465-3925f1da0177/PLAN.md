---
name: Batch 4: Testing & Stability
description: Architecting and implementing a robust C++ unit testing suite for ConfigManager, DataManager, and SystemManager, including build-time constant audits and unified mocking with the layout simulator.
created: 2026-03-29T18:11:00Z
status: SAVED
---

# Batch 4: Testing & Stability

This plan covers the implementation of the firmware unit testing suite using Unity and the `unit_testing_host` environment.

## Context
The project requires a more robust way to verify logic changes across multiple boards and complex configuration states. This plan bridges the gap between the existing WebAssembly-based layout simulator and the formal C++ unit tests.

## Key Goals
- **Unified Mocks**: Share the same hardware abstractions between tests and the simulator.
- **Logic Injection**: Allow real-time and automated testing of WiFi, Weather, and OTA states.
- **Data Snapshots**: Use real API responses for high-fidelity testing.
- **Build Audit**: Ensure all environments are tuned for their respective hardware footprints.
