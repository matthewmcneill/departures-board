# Execution Tasks

- [ ] **Phase 1: Configuration Support**
  - [ ] Update `configManager.hpp` with `ScheduleRule` struct.
  - [ ] Add `schedules` array, `manualOverrideTimeoutSecs`, and `carouselIntervalSecs` to `Config`.

- [ ] **Phase 2: Core Logic (`schedulerManager`)**
  - [ ] Create `schedulerManager.hpp` with `isManualOverrideActive`, `overrideTimestamp`, `triggerManualOverride()`, and `getActiveBoards()`.
  - [ ] Create `schedulerManager.cpp` and implement local time rule evaluations.
  - [ ] Encapsulate the empty board check (`complete == false`).

- [ ] **Phase 3: Integration**
  - [ ] Update `systemManager.cpp` to map physical button presses to `triggerManualOverride()`.
  - [ ] Update `displayManager.cpp` to replace unconditional rotation with the `getActiveBoards()` query.
  - [ ] Update `displayManager::cycleNext()` to use `carouselIntervalSecs`.

- [ ] **Phase 4: Web Portal**
  - [ ] Restructure `#tab-schedule` in `web/index.html` to match the Mobile-First wireframe.
  - [ ] Implement JS `saveAll()` serialization for the `ScheduleRule` array.
  - [ ] Add the "Target Missing" warning state for orphaned rules.

- [ ] **Phase 5: Verification**
  - [ ] Run Playwright tests on Web UI.
  - [ ] Verify manual override logic and timeout behavior on ESP32 device.
