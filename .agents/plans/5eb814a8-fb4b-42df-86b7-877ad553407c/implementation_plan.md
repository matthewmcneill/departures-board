# Boot Progress Refactor

[x] Reviewed by House Style - passed
[x] Reviewed by Architectural - passed
[x] Reviewed by Embedded Systems - passed
[x] Reviewed by UI Design - passed (mockup added)


The objective is to fix the disjointed and confusing boot sequence progress bar. Currently, the progress goes from 0-100% during the synchronous setup in `appContext::begin()`. Afterward, it visually jumps backwards when the system clock is synchronized, and entirely misses the asynchronous data pre-load progress because the `systemManager::firstLoad` flag is dropped prematurely due to a logical deadlock structure. This plan unifies the progress pipeline across `begin()`, `tick()`, and the `systemManager` callbacks.

## UI Design Mockup

```text
+------------------------------------------+
|          DEPARTURES BOARD v3             |
|                                          |
|        [==============>          ]       |
|                 45%                      |
|                                          |
|       Synchronizing System Clock...      |
+------------------------------------------+
```
*The progress bar now fills linearly from 0% (Power On) to 100% (First Data Received).*


## User Review Required

> [!NOTE]
> Modifying `appContext::tick()` states can touch the core system lifecycle. A review ensures the proposed `AppState` flow remains consistent with the intended design.

## Proposed Changes

### appContext

We will map the synchronous boot sequence down to 0-30%, and the system clock sync to 30-60%. We will also slightly restructure the `AppState::BOOTING` transition to ensure `WIFI_SETUP` and `BOARD_SETUP` modes immediately supersede the `firstLoad` wait.

#### [MODIFY] appContext.cpp (modules/appContext/appContext.cpp)
- **`appContext::begin()`**: Remap `updateBootProgress` percentages down to scale between `5-30%` (e.g., 5, 10, 15, 20, 25, 30).
- **`appContext::tick()`**:
  - Update `timeManager.initialize` loop variables. Start `progress = 30` and loop from `30` to `60` (instead of 50-80).
  - Modify the logic that evaluates `WIFI_SETUP` and `BOARD_SETUP` to happen *before* waiting for `firstLoad`.
  - Add logic to monitor `networkManager.getNoDataLoaded()` and drop `firstLoad = false` once data is present.
  - Fallback: if `config.boardCount == 0`, instantly drop `firstLoad = false`.

### dataManager

We will ensure the data fetch cycle correctly reports its first success to clear the loading screen.

#### [MODIFY] dataManager.cpp (modules/dataManager/dataManager.cpp)
- **`dataManager::workerTaskLoop`**: Inside the successful fetch block (after `targetToExecute->executeFetch()`), set `noDataLoaded = false`.


## Resource Impact Assessment

### Memory (Flash / RAM)
- **Flash**: Negligible (~24 bytes for `setFirstLoad` method and logic branch).
- **RAM**: Zero additional static allocation.
- **Stack**: Negligible impact on `tick()` and `begin()` stack frames.
- **Heap**: **CRITICAL**: Zero heap fragmentation. This refactor uses existing stack-based integer math.

### Power & Timing
- **Power**: No change to duty cycle or sleep modes.
- **Timing**: Resolves a logical deadlock in `tick()` that previously caused one wasted cycle before the first data fetch.


## Verification Plan

### Automated Tests
*None available. This is a framework-level lifecycle state refactor.*

### Manual Verification
1. Power cycle the device.
2. Observe the initial loading screen progress goes to ~30%.
3. Observe "Waiting for WiFi" or the clock sync proceeds to loop up to ~60%.
4. Observe the final gap `60% -> 100%` smoothly represents the initial data-fetching cycle.
5. The device seamlessly transitions into `RUNNING` mode and shows the primary interface at exactly 100%.
