---
name: v3.1 Library Modernization & Configuration Migration Debug
description: Resolve library deprecations and debug configuration migration firmware.
created: 2026-04-11T16:51:11Z
status: DONE
commits: ["91c3b8ae4030b5d74c2f725f996a487911e22a4b", "eae03d09a80e668ffbcad5ceac684260aa74d538"]
---

# Summary
This plan resolves 14+ compiler warnings caused by deprecated methods in `ArduinoJson` and `ESPAsyncWebServer`. It upgrades project dependencies (`U8g2`, `ArduinoJson`, `ESPAsyncWebServer`) to their latest maintained versions. Additionally, it implements on-device MCP diagnostic tools (`list_files`, `get_file_raw`) to successfully debug and verify the v2.6 `ConfigManager` nested schema migration logic on physical hardware.

## Technical Context
- [Context Bridge](context_bridge.md): Distilled technical findings.
- [Sessions](sessions.md): Complete session history.
- [Task List](task.md): Progress checklist.
