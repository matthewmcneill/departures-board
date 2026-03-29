# Project TODO: Logical Batches

The following open tasks from `.agents/todo_list.md` have been grouped into logical batches to focus future development sessions.

## 📦 Batch 1: Web Portal UX & Security
*Focus: Refining the user-facing settings, API key security, and input validations.*
- Review password masking (grey stars).
- 'X' close button for keys form.
- API key save logic (commit if unchanged).
- API key format validation.
- Link/instructions for keys.
- Rail Station field CRS resolution.
- Lat/Long fields in Advanced Options.
- Tooltips for "No Scrolling" / "Fast Refresh".
- Filter string validation in portal.

## 📦 Batch 2: Diagnostics & Telemetry
*Focus: Enhancing system visibility, watchdog metrics, and hardware status reporting.*
- TWDT (Watchdog) metrics UI.
- Background task states.
- Real-time Background Polling Log.
- Max heap size in system page.
- Real-time OLED power state (web UI).
- `temperatureRead()` accuracy audit.

## 📦 Batch 3: Firmware Infrastructure
*Focus: Implementing the OTA state machine and migrating legacy scheduling logic.*
- OTA machine state in `appContext`.
- OTA wizard/progress bar UI.
- OTA widget (remove overlay board).
- Schedule Migration (v1/v2 config parsing).
- Data expiry logic in `iDataProviders`.
- RSS feedback timeout durations.
- Refresh rate as per-key property.

## 📦 Batch 4: Testing & Stability
*Focus: Building the C++ unit testing suite for core managers and data fetchers.*
- Unit tests for `ConfigManager::hasConfiguredBoards()`.
- Audit `departuresBoard.hpp` (pio variables, max keys).
- Unit tests for `dataManager` priority queue.
- Unit tests for `SystemManager` Round-Robin.
- `unit_testing_host` for `drawText`.
- `ConfigManager` migration scenario tests.
- `determineInitialTab` verification suite.
- `WebServer::handleClient()` re-entrancy unit test.

## 📦 Batch 5: Simulator (WASM) Parity
*Focus: Fixing the WebAssembly layout simulator to match firmware behavior.*
- WASM: Populate `location` array (National Rail fix).
- WASM: Handle `oledOff` simulation.
- WASM: Mark omitted widgets as `isVisible = false`.
