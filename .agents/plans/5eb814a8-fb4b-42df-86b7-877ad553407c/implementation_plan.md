# Boot Progress Refactor

The objective is to fix the disjointed and confusing boot sequence progress bar. Currently, the progress goes from 0-100% during the synchronous setup in `appContext::begin()`. Afterward, it visually jumps backwards when the system clock is synchronized, and entirely misses the asynchronous data pre-load progress because the `systemManager::firstLoad` flag is dropped prematurely due to a logical deadlock structure. This plan unifies the progress pipeline across `begin()`, `tick()`, and the `systemManager` callbacks.

## User Review Required

> [!NOTE]
> Modifying `appContext::tick()` states can touch the core system lifecycle. A review ensures the proposed `AppState` flow remains consistent with the intended design.

## Proposed Changes

### appContext

We will map the synchronous boot sequence down to 0-30%, and the system clock sync to 30-60%. We will also slightly restructure the `AppState::BOOTING` transition to ensure `WIFI_SETUP` and `BOARD_SETUP` modes immediately supersede the `firstLoad` wait.

#### [MODIFY] appContext.cpp (modules/appContext/appContext.cpp)
- **`appContext::begin()`**: Remap `updateBootProgress` percentages down to scale between `10-30%` (e.g., 5, 10, 15, 20, 25, 30).
- **`appContext::tick()`**:
  - Update `timeManager.initialize` loop variables. Start `progress = 30` and loop from `30` to `60` (instead of 50-80).
  - Modify the logic that evaluates `WIFI_SETUP` and `BOARD_SETUP` to happen *before* waiting for `sysManager.getFirstLoad()`. If it falls through to normal board booting, it will then block `AppState::RUNNING` until `!sysManager.getFirstLoad()` is satisfied.
  - Expose a `setFirstLoad` method in `systemManager` to forcefully clear it if intercepting WiFi/Setup Wizards.

### systemManager

We will fix the deadlock that currently completely hides data fetching progress.

#### [MODIFY] systemManager.hpp (modules/systemManager/systemManager.hpp)
- Add `void setFirstLoad(bool state)` to control the `firstLoad` status externally if `appContext` enters a setup wizard.

#### [MODIFY] systemManager.cpp (modules/systemManager/systemManager.cpp)
- **`systemManager::tick()`**:
  - Update `if (millis() > nextRoundRobinUpdate && wifiConnected && context->getAppState() == AppState::RUNNING)` to evaluate `(context->getAppState() == AppState::RUNNING || firstLoad)`. This allows the initial data fetch to proceed while `appContext` is natively preserving the loading board in `BOOTING` mode.
  - Reposition the `firstLoad = false` setter. Currently it lives outside the fetch result logic. Move it into the `if (lastUpdateResult == 0 || lastUpdateResult == 1)` block, directly under `displayMgr.render()`.
  - Fallback: if `config.boardCount == 0`, instantly drop `firstLoad = false`.
- **`systemManager::tflCallback()`**: Update incremental steps to scale between 60-80%.
- **`systemManager::raildataCallback()`**: Update calculation to scale from 80-100%.

## Verification Plan

### Automated Tests
*None available. This is a framework-level lifecycle state refactor.*

### Manual Verification
1. Power cycle the device.
2. Observe the initial loading screen progress goes to ~30%.
3. Observe "Waiting for WiFi" or the clock sync proceeds to loop up to ~60%.
4. Observe the final gap `60% -> 100%` smoothly represents the initial data-fetching cycle.
5. The device seamlessly transitions into `RUNNING` mode and shows the primary interface at exactly 100%.
