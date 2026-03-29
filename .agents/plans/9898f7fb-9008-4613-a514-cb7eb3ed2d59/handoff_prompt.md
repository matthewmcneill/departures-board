# Batch 4: Testing & Stability - Handoff Prompt

## 🎯 Primary Objective
Architect and implement a robust C++ unit testing suite for the `departures-board` firmware, focusing on the `ConfigManager`, `DataManager`, and `SystemManager` modules. Conduct an audit of build-time constants to ensure environment parity.

---

## 🏗️ Architectural Context
- **Hardware**: ESP32 / ESP32-S3 (Arduino framework).
- **Core Orchestrator**: `appContext` (Dependency Injection container).
- **Testing Framework**: [Unity](https://github.com/ThrowTheSwitch/Unity) via PlatformIO.
- **Environment**: `unit_testing_host` (Desktop-based C++ simulation).
- **House Style**: `camelCase` naming, `.hpp` headers, and Doxygen-style documentation.

---

## 🛠️ Specific Tasks & Verification Targets

### 1. ConfigManager & Migration Logic
- **Target**: `modules/configManager/configManager.cpp`
- **Tests**:
    - `hasConfiguredBoards()`: Verify it returns `false` if boards have default/incomplete fields, forcing the `BOARD_SETUP` state.
    - **Migration Scenario**: Simulate a v2.3 `config.json` (using global `turnOffOledInSleep`) and verify it correctly migrates to v2.4+ (per-board `oledOff` property).
- **Goal**: prevent "skipping setup" and ensure zero-loss configuration upgrades.

### 2. DataManager Priority Queue
- **Target**: `modules/dataManager/`
- **Tests**:
    - Verify that connection timeouts trigger a retry with appropriate priority.
    - Ensure duplicate fetch requests (multiple boards sharing one key/station) are deduplicated correctly.
- **Goal**: Optimize WiFi usage and prevent redundant API hits.

### 3. SystemManager Round-Robin
- **Target**: `modules/systemManager/`
- **Tests**:
    - Monitor data load distribution across 6+ boards.
    - Verify that the update multiplexing queue never starves a specific board index.
- **Goal**: Ensure equitable refresh rates for multi-display setups.

### 4. Hardware Abstraction & `drawText`
- **Target**: `modules/displayManager/drawingPrimitives.cpp`
- **Tests**:
    - Host-based validation of `drawText` fast-path logic.
    - Verify "Rough-Cut" truncation accuracy under extreme string lengths (preventing buffer overruns or weird rendering).
- **Goal**: Stabilize UI rendering logic without needing physical hardware for every tweak.

### 5. Build-Time Audit
- **Files**: `src/departuresBoard.hpp` vs `platformio.ini`
- **Task**: Compare all `#define` defaults in the header against the `build_flags` in `platformio.ini`. Ensure `MAX_KEYS` (ideally 4-8) and `MAX_BOARDS` (6) are consistent.

---

## 🚀 Execution Instructions
1. **Explore**: Review `test/test_native/test_main.cpp` for the existing Unity structure.
2. **Build**: Use `pio test -e unit_testing_host` to run tests locally on your machine.
3. **Draft**: Create new test files in `test/test_native/` for each module.
4. **House Style**: Ensure all new code adheres to the `.agents/skills/house-style-docs/SKILL.md` rules.

---

## 📋 Done Criteria
- [ ] `pio test -e unit_testing_host` passes with 100% success.
- [ ] `departuresBoard.hpp` is synchronized with `platformio.ini`.
- [ ] `ConfigManager` migration logic is formally verified via JSON test vectors.
- [ ] All new tests follow the project's architectural standards (SRP/DIP).
