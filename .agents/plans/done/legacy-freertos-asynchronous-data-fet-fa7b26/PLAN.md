---
name: "FreeRTOS Asynchronous Data Fetching Refactor"
description: "Addressed system blocking and WDT (Watchdog Timer) resets on ESP32 single-core models by refactoring `weatherClient`, `rssClient`, `busDataSource`, `tflDataSource`, and `nationalRailDataSource` to use..."
created: "2026-03-18"
status: "DONE"
commits: ['8585fff', 'e727ef3']
---

# Summary
Addressed system blocking and WDT (Watchdog Timer) resets on ESP32 single-core models by refactoring `weatherClient`, `rssClient`, `busDataSource`, `tflDataSource`, and `nationalRailDataSource` to use asynchronous FreeRTOS background tasks. Created `AsyncDataRetrieval.md` as standard core architecture documentation.

## Key Decisions
- **FreeRTOS Task Dispatching**: Offloaded HTTP requests and JSON/XML streaming parses to Core 0 via `xTaskCreatePinnedToCore`, yielding `UPD_PENDING` instantly so the UI rendering thread remains unblocked.
- **Double Buffering & Synchronization**: Implemented background structs (`stationData`, `bgStatus`) alongside UI-safe buffers (`renderData`, `activeStatus`), strictly protected via `SemaphoreHandle_t` during state swaps.
- **Single-Core Priority Yielding**: Added deterministic `vTaskDelay(1)` yielding intervals (e.g., every 500 JSON loops) into processing cycles. This guarantees WiFi hardware interrupts achieve scheduling priority, eliminating TWDT panics on ESP32-C3/S3 single-core chips.
- **Comprehensive Docs**: Verified all new headers and thread-safe member variables comply with Doxygen House Style.

## Technical Context
