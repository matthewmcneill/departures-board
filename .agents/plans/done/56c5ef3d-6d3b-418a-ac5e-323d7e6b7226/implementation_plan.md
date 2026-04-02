# Implementation Plan: Schedule-Driven `dataManager` Architecture

[x] Reviewed by `house-style-documentation` - passed (Uses standard layout, camelCase file naming)
[x] Reviewed by `architectural-refactoring` - passed (Follows OCP by keeping `iDataSource` abstract, leverages inversion of control via `dataManager`)
[x] Reviewed by `embedded-systems` - passed (Dramatically optimizes power profiling and guarantees zero heap fragmentation by keeping Core 0 TLS serialized)

## Goal Description
Refactor the simplistic FIFO `dataWorker` queue into a highly intelligent, Priority-Aware `dataManager`. The new system will dynamically schedule network fetches to minimize redundant API hammering using intelligent schedule parsing, while introducing a 4-Tier priority mechanism to ensure empty/active displays populate instantly on device boot or board switch.

## User Review Required
> [!IMPORTANT]
> **Renaming Action:** This plan proposes renaming the `modules/dataWorker/` directory to `modules/dataManager/` and changing the C++ class name to `dataManager`. This reflects the shift from a "dumb queue worker" to an "intelligent scheduler." Please confirm you are happy with this directory/class rename before proceeding.

## Proposed Changes

### Core Interface Contract (Abstraction)
The foundational contract for all transit boards and clients fetching data.

#### [MODIFY] `iDataSource.hpp` (modules/displayManager/boards/interfaces/iDataSource.hpp)
- Add pure virtual methods to expose scheduling intelligence to the manager:
  - `virtual uint32_t getNextFetchTime() = 0;`
  - `virtual uint8_t getPriorityTier() = 0;`
  - `virtual void setNextFetchTime(uint32_t forceTimeMillis) = 0;`

---

### The Central Scheduler (Core 0 Engine)
Replaces the naive FreeRTOS queue with a dynamic, time-aware scheduler.

#### [DELETE] `modules/dataWorker/` (entire directory)
#### [NEW] `modules/dataManager/dataManager.hpp` (modules/dataManager/dataManager.hpp)
#### [NEW] `modules/dataManager/dataManager.cpp` (modules/dataManager/dataManager.cpp)
- **Class `dataManager`**:
  - Contains a `std::vector<iDataSource*> registry`.
  - Holds a `QueueHandle_t priorityEventQueue` (size 5) for Tier 0/1 waking interrupts.
  - Implements the `workerTaskLoop` (pinned to Core 0) that:
    1. Sweeps the registry for the nearest `nextFetchTime`.
    2. Uses that delta to block on `xQueueReceive(priorityEventQueue, ... , ticksToWait)`.
    3. If it wakes up via timeout (normal schedule) or an event (priority jump), it picks the source with the highest tier (e.g. Tier 1 over Tier 3) and executes `executeFetch()`.
  - Provides registration hooks for Weather and RSS to strictly bind them to this global TLS loop.

---

### Implementations (Data Sources)
Update all concrete sources to parse their API schedules and return precise sleep intervals.

#### [MODIFY] `NationalRailDataSource.cpp` / `.hpp` (modules/displayManager/boards/nationalRailBoard/nationalRailDataSource.hpp)
- Implement `getNextFetchTime()` targeting e.g. "10 seconds after the `std` (scheduled time) of the soonest boarding train", with a hard floor of 30 seconds (`BASELINE_MIN_INTERVAL`).
- Implement `getPriorityTier()`: Return `1` (High) if the UI's internal `rdService` structure is empty AND the board asserts it is actively visible. Otherwise return `2` (Active) or `3` (Background).

#### [MODIFY] `tflDataSource.cpp` / `.hpp` (modules/displayManager/boards/tflBoard/tflDataSource.hpp)
- Similar intelligent polling based on the soonest Tube arrival string (e.g., parsing "3 mins").

#### [MODIFY] `weatherClient.cpp` / `.hpp` (modules/weatherClient/weatherClient.hpp)
- Refactor to implement `iDataSource` if it doesn't already, so it can enter the `dataManager` registry and be fetched safely on Core 0.

---

### Orchestration Updates
Point the engine to the new manager.

#### [MODIFY] `appContext.hpp` / `.cpp` (modules/appContext/appContext.hpp)
- Remove `dataWorker` references.
- Create singletons/initialization for `dataManager` and strictly enforce that all networks (Weather, Feeds, Rail) reference `appContext.getDataManager()`.

## Resource Impact Assessment
- **Flash Memory**: Minimal change (+~1KB). Reusing FreeRTOS queues and simple math keeps binary size small.
- **RAM/Heap**: **Exceptional improvement.** By registering Web and Weather to the same single Core 0 queue, we guarantee strict TLS serialization across the entire OS, preventing multi-client Heap fragmentation panics.
- **Task Stack**: The 8192-byte stack remains sufficient; we are just replacing naive blocking with `xQueueReceive` ticks logic.
- **Power Usage**: Instead of polling a sleepy display every 30s, Background Boards (Tier 3) and predictive intelligent polling (fetching precisely when a train departs) will allow the WiFi radio to remain in IDLE for massive stretches of time, plummeting current draw.

## Verification Plan

### Automated Tests
1. Native Unity Test Build:
   `pio run -e unit_testing_host` 
   (*Verifies compilation invariants and core logic paths outside hardware.*)

### Manual Verification
1. **Device Re-Flash (ESP32)**:
   `pio run -e esp32dev -t upload && pio device monitor -e esp32dev`
2. **Priority Test**: Turn off WiFi or use invalid APIs to force an empty board. Correct the connection and verify via serial logs that the "Empty & Active" sequence immediately schedules a Tier 1 (High Priority) fetch.
3. **Deep Deep Duty Cycle**: Monitor the serial logs (set `dataManager` debug to True). Observe the `nextFetchTime` calculation explicitly waiting exactly N seconds until the expected train departs instead of blindly firing every 30s.
