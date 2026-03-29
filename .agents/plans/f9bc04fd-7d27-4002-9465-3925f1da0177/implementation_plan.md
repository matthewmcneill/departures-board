# Implementation Plan - Batch 4: Testing & Stability

This plan outlines the architecture and implementation of a robust C++ unit testing suite for the departures-board firmware, alongside a build-time constant audit to ensure environment parity.

## Context
Standardizing the quality of implementation plans ensures that all proposed changes are consistent with the project's technical goals, naming conventions, and resource constraints (especially important for ESP32).

## Audit Checklist
- [x] Reviewed by `house-style-docs` - passed (renamed tests to camelCase)
- [x] Reviewed by `oo-expert` - passed (DIP/SRP maintained via interfaces/modular tests)
- [ ] Reviewed by `embedded-web-designer` - N/A (No UI changes)
- [x] Reviewed by `embedded-systems` - passed (Resource Impact Assessment added)

## Relationship to Layout Simulator (displaysim)

Both the **Test Rig** (`unit_testing_host`) and the **Layout Simulator** (`layoutsim`) require extensive library stubbing to execute ESP32/Arduino code on desktop platforms. 

- **Crossover**: The `layoutsim` already contains a comprehensive set of mocks in `tools/layoutsim/src/` specifically for `Arduino.h`, `LittleFS.h`, and `WiFi.h`.
- **Strategy**: Rather than creating a redundant second set of stubs, I will extract and unify these mocks into a central `test/mocks/` directory. This ensures that the logic behavior in tests matches the visual behavior in the simulator.

## Proposed Changes

### [Mocks] [NEW] [test/mocks/](file:///Users/mcneillm/Documents/Projects/departures-board/test/mocks/)
Unify and adapt existing `layoutsim` mocks for both Native and WASM environments.
- **Shared Arduino Stubs**: Capture `String`, `Print`, `millis()`, and `PROGMEM` macros from `tools/layoutsim/src/Arduino.h`.
- **Logic Injection**: Enhance `LittleFS` and `WiFi` mocks to allow programmatic state injection (e.g., simulating a missing config file or a network timeout) during unit tests.
- **U8G2 Graphics Stubs**: Leverage the current `layoutsim` virtual buffer to allow `drawingPrimitives` tests to verify pixel-level truncation without a physical screen.

---

### [ConfigManager]
#### [MODIFY] [configManager.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/configManager/configManager.cpp)
- Enhance migration logic to handle legacy `turnOffOledInSleep` (v2.3).
- **Migration Rule**: If `turnOffOledInSleep` is found and true, scan the `boards` array for any board of type `MODE_CLOCK`. Set `oledOff = true` for those boards.
#### [NEW] [configManagerTests.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/test/test_native/configManagerTests.cpp)
- Test `hasConfiguredBoards()` with various board states.
- Test migration logic using JSON blobs for v2.3 and v2.4.
- Verify `save()` and `load()` consistency using the `test/mocks/LittleFS` stub.

---

### [DataManager]
#### [NEW] [dataManagerTests.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/test/test_native/dataManagerTests.cpp)
- Test `requestPriorityFetch` correctly elevates a source to the front of the queue and updates `nextFetchTime`.
- Verify `workerTaskLoop` (simulated) picks the highest tier source first.
- Test deduplication: Register two sources with identical station/key and verify they result in expected fetch behavior.

---

### [SystemManager]
#### [NEW] [systemManagerTests.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/test/test_native/systemManagerTests.cpp)
- Simulate a multi-board setup (6+ boards).
- Verify `tick()` correctly increments `backgroundUpdateIndex` in a round-robin fashion.
- Ensure `distributedInterval` math results in equitable refresh rates.

---

### [DisplayManager/Drawing]
#### [NEW] [drawingPrimitivesTests.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/test/test_native/drawingPrimitivesTests.cpp)
- Validate `drawText` "Rough-Cut" truncation logic.
- Verify pixel-width calculations (mocked) result in correct truncation points.
- Test fast-path vs full-path execution flow.

---

### [Build Audit]
#### [MODIFY] [departuresBoard.hpp](file:///Users/mcneillm/Documents/Projects/departures-board/src/departuresBoard.hpp)
- Standardize defaults using `#ifndef` blocks.
- Define `MAX_KEYS` default (8).
- Ensure `MAX_BOARDS` default (6).
- This allows `platformio.ini` to override these via `build_flags` for smaller boards (e.g., `esp32s3nano`) to reduce RAM footprint.
#### [MODIFY] [platformio.ini](file:///Users/mcneillm/Documents/Projects/departures-board/platformio.ini)
- Explicitly set `MAX_KEYS` and `MAX_BOARDS` values for each production environment.
- For `esp32s3nano`, keep `MAX_KEYS=4` to conserve memory.
- For `esp32dev`, use `MAX_KEYS=8` for full capacity.
- Update `unit_testing_host` to define these constants to prevent compilation errors.

## Resource Impact Assessment

### Memory (Flash / RAM / Stack / Heap)
- **Flash**: Adding migration logic in `configManager.cpp` will increase binary size by ~200-400 bytes. Unit tests only run on host, so no impact on ESP32 Flash for the test binaries themselves.
- **RAM**: No significant change to global variables.
- **Stack/Heap**: `drawText` truncation logic uses a 256-byte stack buffer (already present). DataManager tests will exercise the FreeRTOS queue, which consumes heap on hardware (already implemented).

### Power
- No impact. The changes are for testing and config migration, which happens once at boot or upon config save.

### Security
- **API Key Redaction**: Migration logic must ensure that legacy keys are also registered for redaction in the Logger. This is handled by `loadApiKeys()` which will be called after migration.

## Open Questions

1. **Test Runner**: Should I consolidate all tests into `test_main.cpp` or keep them modular as separate files in `test/test_native/`? Modularity is preferred for Unity, but requires a coordinator. **Decision**: I'll keep them modular and call them from `test_main.cpp`.
2. **Migration Logic**: If `turnOffOledInSleep` was `true` in v2.3, where should it be migrated in v2.4? **Decision**: It will be migrated into the `oledOff` property of any boards of type `MODE_CLOCK` (Screensaver). This ensures the display turns off only when the sleep/screensaver context is active, matching your design pattern where sleep only triggers if a clock board is available.

## Verification Plan

### Automated Tests
- `pio test -e unit_testing_host`
- This will execute all Unity-based tests on the host machine. 100% pass rate is required.

### Manual Verification
- Audit `platformio.ini` against `departuresBoard.hpp` to ensure no mismatches remain.
- Run a `pio run` build for `esp32dev` to ensure the core code still compiles for hardware after migration logic updates.
