---
name: Stabilizing Native Test Suite
description: Resolving native linker scoping collisions and ArduinoFake C++ exceptions to pass unit tests in unit_testing_host.
created: 2026-04-26T22:33:00Z
status: WIP
commits: []
---

# Summary
This plan aims to execute and stabilize the native host test environment for departures-board by resolving global state linker collisions (specifically `appContext`) and rectifying hardware stub implementations (MockLittleFS bypassing ArduinoFake exceptions). It was safely yielded while dealing with a SIGABRT immediately following the `test_drawText_truncation` runner.

## Technical Context
- [Context Bridge](context_bridge.md): Distilled technical findings.
- [Sessions](sessions.md): Complete session history.
- [Task List](task.md): Progress checklist.
