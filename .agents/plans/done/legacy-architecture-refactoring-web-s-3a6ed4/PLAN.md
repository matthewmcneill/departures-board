---
name: "Architecture Refactoring, Web Stability & Dynamic Layouts"
description: "Successfully implemented Dynamic Layout Selection allowing per-board assignment of distinct visual templates (e.g., \"Default\" vs \"Replica\") directly via the Web Portal. Stabilized system operations by..."
created: "2026-03-28"
status: "DONE"
commits: ['beb71e7']
---

# Summary
Successfully implemented Dynamic Layout Selection allowing per-board assignment of distinct visual templates (e.g., "Default" vs "Replica") directly via the Web Portal. Stabilized system operations by diagnosing and mitigating `Task Watchdog Timer (TWDT)` crashes inside the HTTP pipeline, and repaired weather condition synchronization to prevent premature network backoffs during the boot cycle. Included custom compiled aesthetic upgrades to U8G2 weather and WiFi fonts.

## Key Decisions
- **Dynamic Board Layouts**: Expanded the `BoardConfig` schema across `configManager` to securely persist the `layout` property. Updated the frontend `index.html` UI with a new dropdown selector and ensured symmetric configuration via `handleSaveAll()` and `handleGetConfig()`.
- **WDT Polling Immunity**: Integrated explicit `esp_task_wdt_reset()` checks into the synchronized Web Server diagnostic loops (`handleTestBoard`, `handleTestKey`), fundamentally repairing the `async_tcp` (Core 1) panics occurring during SSL handshake timeouts.
- **Boot Sequence Validation**: Fixed a debilitating race condition inside `weatherClient` where incomplete `WeatherStatus` properties triggered forced 15-second data polling backoffs.
- **Custom Font Recompilation**: Seamlessly integrated user modifications to ASCII source blocks (`WeatherIcons11.txt`, `WifiIcons11.txt`) into immutable `fonts.cpp` binaries via `build_fonts.py`.

## Technical Context
