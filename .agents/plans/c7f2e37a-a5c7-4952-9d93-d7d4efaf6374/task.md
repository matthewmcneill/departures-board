# Execution Tasks

- [x] **Phase 1: Configuration Support**
  - [x] Update `configManager.hpp` with `ScheduleRule` struct.
  - [x] Add `schedules` array, `manualOverrideTimeoutSecs`, and `carouselIntervalSecs` to `Config`.

- [x] **Phase 2: Core Logic (`schedulerManager`)**
  - [x] Create `schedulerManager.hpp` with `isManualOverrideActive`, `overrideTimestamp`, `triggerManualOverride()`, and `getActiveBoards()`.
  - [x] Create `schedulerManager.cpp` and implement local time rule evaluations.
  - [x] Encapsulate the empty board check (`complete == false`).

- [x] **Phase 3: Integration**
  - [x] Update `systemManager.cpp` to map physical button presses to `triggerManualOverride()`.
  - [x] Update `displayManager.cpp` to replace unconditional rotation with the `getActiveBoards()` query.
  - [x] Update `displayManager::cycleNext()` to use `carouselIntervalSecs`.

- [ ] **Phase 4: Web Portal**
  - [ ] Restructure `#tab-schedule` in `web/index.html` to match the Mobile-First wireframe.
  - [ ] Implement JS `saveAll()` serialization for the `ScheduleRule` array.
  - [ ] Add the "Target Missing" warning state for orphaned rules.

- [ ] **Phase 5: Verification**
  - [ ] Run Playwright tests on Web UI.
  - [ ] Verify manual override logic and timeout behavior on ESP32 device.
