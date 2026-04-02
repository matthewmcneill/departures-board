# Implementation Plan - Boot Progress Bar Refactor (Refined)

[x] Reviewed by House Style - Matt McNeill
[x] Reviewed by Architectural - Matt McNeill
[x] Reviewed by Embedded Systems - Matt McNeill
[x] Reviewed by UI Design - Matt McNeill

This plan addresses the disjointed boot sequence where progress jumps backwards and the system remains stuck on the loading screen due to a missing `firstLoad` state transition. We will unify the progress reporting to be a contiguous 0-100% linear flow across the entire boot lifecycle.

## User Review Required

> [!IMPORTANT]
> The progress bar segments are being redefined. If specific hardware initialization steps take significantly longer than others, we may need to adjust these weights later.
> 
> [!WARNING]
> We are introducing a dependency where the UI will not transition to `RUNNING` until at least one data source has successfully fetched data. This ensures the user never sees an empty board, but requires a working network connection for a "clean" boot.

## Proposed Changes

### appContext (modules/appContext)
The central hub will now manage the "High Level" progress state, aggregating data from sub-managers.

#### [MODIFY] [appContext.cpp](modules/appContext/appContext.cpp)
- **`appContext::begin()`**: Remap synchronous steps to **0-25%**.
    - Initialize display/splash: **0-5%**
    - LittleFS/Config: **5-15%**
    - Wifi/Network Init: **15-20%**
    - Registration: **20-25%** (End of synchronous boot).
- **`appContext::tick()`**:
    - **Phase 2 (25-50%)**: While `!wifiManager.isReady()`, maintain progress at 25-50% based on WiFi connection status.
    - **Phase 3 (50-75%)**: Remap `timeManager.initialize` loop to start at 50% and go up to 75%. Remove the "loop back to 45" logic.
    - **Phase 4 (75-100%)**: Once clock is synced, signal `networkManager` to trigger initial fetches. Progress will move from 75% to 100% as `networkManager.getNoDataLoaded()` becomes false.
    - **State Transition**: Ensure `firstLoad` is set to `false` ONLY when `networkManager.getNoDataLoaded()` is false OR if no boards are configured.

### dataManager (modules/dataManager)
The background worker needs to correctly flag when the first successful data load occurs.

#### [MODIFY] [dataManager.cpp](modules/dataManager/dataManager.cpp)
- **`dataManager::workerTaskLoop`**: 
    - Inside the `if (targetToExecute != nullptr)` block, after `targetToExecute->executeFetch()`.
    - We must verify the fetch was successful (e.g., check `targetToExecute->getLastErrorMsg()` or similar, but the `iDataSource` implementation should ideally report back).
    - For now, we will update `noDataLoaded = false` when *any* fetch completes without a FATAL error (or just any fetch for simplicity in this first load phase).
    - **Action**: Increment `dataLoadSuccess` properly and update `noDataLoaded` based on the fetch results.

## UI Design Mockup

```text
+------------------------------------------+
|          DEPARTURES BOARD v3             |
|                                          |
|        [==================>      ]       |
|                 75%                      |
|                                          |
|         Fetching Station Data...         |
+------------------------------------------+
```
*Linear 0-100% progress covering: HW Init (25%) -> WiFi/NTP (50%) -> First Data (25%)*

## Resource Impact Assessment

### Memory (Flash / RAM)
- **Flash**: Minimal (< 100 bytes) for remapped constants and logic branches.
- **RAM**: No new allocations. Using existing `appContext` and `dataManager` fields.

### Power & Timing
- **Timing**: Prevents the "deadlock" where the UI waits forever for `firstLoad = false`.
- **UX**: Eliminates the jarring 100% -> 50% progress jump.

## Verification Plan

### Automated Tests
- None (Lifecycle behavior is hardware-dependent).

### Manual Verification
1. **Normal Boot**: Power cycle and verify progress bar moves forward monotonically until the boards appear.
2. **No Config Boot**: Clear config, verify it transitions to `BOARD_SETUP` help screen after NTP sync (since no data can be fetched).
3. **No WiFi Boot**: Verify it stays at the WiFi connection phase (~25-50%) until timeout or connection.
