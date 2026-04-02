# Dismantle `systemManager` "God Object"

This plan details the full dismantling and removal of `systemManager` to address the "God Object" anti-pattern and strictly adhere to the project's Single Responsibility Principle and Open/Closed Principle. Functionality is relocated to tightly scoped domain leaders.

## Proposed Changes

---

### `appContext` (State Machine & Boot Sequence)
Migrate the startup, lifecycle, and soft-reset components into the primary state orchestration engine. `appContext` will absorb the UI dispatching parts of the boot sequence.

#### [MODIFY] appContext.hpp
- **Add variables:** 
  - `bool firstLoad`
  - `int startupProgressPercent`
  - `int prevProgressBarPosition`
- **Add methods:**
  - `void softResetBoard()`: Performs the reconfiguration soft reboot.
  - `String getBuildTime()`: Helper for boot logging.

#### [MODIFY] appContext.cpp
- Integrate `softResetBoard()` and `getBuildTime()` implementations.
- Refactor how the inline lambdas were passed to `systemManager`. As `appContext` now directly owns progress state variables (`startupProgressPercent` etc), it can invoke logging directly without lambda indirection.

---

### `wifiManager` (Network State)
Consolidate network status and IP formatting logic natively into the `WifiManager`.

#### [MODIFY] wifiManager.hpp
- **Add variables:** Keep track of prolonged downtime.
  - `unsigned long wifiDisconnectTimer` 
  - `char myUrl[24]`
- **Add methods:**
  - `void updateMyUrl()`
  - `const char* getMyUrl() const`
  - `bool isWifiPersistentError() const`

#### [MODIFY] wifiManager.cpp
- Wire `updateMyUrl()` to trigger when WiFi transitions to `WIFI_READY` inside `tick()`.
- Record timestamp to `wifiDisconnectTimer` when WiFi disconnects to properly serve `isWifiPersistentError()`.

---

### `rssClient` (RSS Feed Logic)
Migrate message pool string building routines locally to the RSS plugin.

#### [MODIFY] rssClient.hpp
- Forward declare or include necessary dependencies to manipulate the `MessagePool`.
- **Add methods:**
  - `void updateRssFeed()`
  - `void addRssMessage(class MessagePool& pool, const struct Config& config)` (Pass dependencies structurally to prevent cycles)
  - `void removeRssMessage(class MessagePool& pool)`

#### [MODIFY] rssClient.cpp
- Migrate implementations of `updateRssFeed()`, `addRssMessage()`, and `removeRssMessage()`.

---

### `dataManager` & Data Sources (Polling & Orphaned Callbacks)
Migrate general background fetching state trackers to `dataManager`, and shift domain-specific UI callback routines to the data sources.

#### [MODIFY] dataManager.hpp & .cpp
- **Add variables:** Move these out of global space for UI polling info:
  - `unsigned long lastDataLoadTime`, `bool noDataLoaded`
  - `int dataLoadSuccess`, `int dataLoadFailure`, `unsigned long lastLoadFailure`, `int lastUpdateResult`, `int lastActiveSlotIndex`
- Update `dataManager` accessors so the UI (like `displayManager` or the Web UI) can poll request statuses.

#### [MODIFY] source clients (e.g., TfL / Rail / Weather)
- The raw `raildataCallback` and `tflCallback` functions in `systemManager` updated the `appContext` `startupProgressPercent` on first boot, and yielded otherwise.
- Move these callbacks cleanly into `appContext` (perhaps leaving them as `yieldCallbackWrapper` adapters that properly consult `firstLoad` state) or inject a structured progress listener into the `dataManager` that delegates to `appContext` on the Main Core.

---

### `systemManager` (Deletion)

#### [DELETE] systemManager.hpp
#### [DELETE] systemManager.cpp

---

### Clean Up & Injection Wiring
- `departuresBoard.cpp` or `main.cpp`: Ensure any straggling references to `sysManager` route to `appContext` or the respective domain singletons.
- Update `appContext.hpp`: Remove `#include "systemManager.hpp"` and remove `sysManager` instance from properties. Update `appContext::tick()` to delete `sysManager.tick()` and handle polling tick logic (such as input handling) internally.
- Migrate button handling (`inputDevice`) directly to `appContext::tick()` since it orchestrates state like toggling `sleep` displays.

## Verification Plan
### Automated Verification
- Verify code compiles without cyclic include errors.
- Confirm compilation using native ESP32 tooling.

### Manual Verification
- Ask the user to verify by flashing and manually validating the startup sequence on the device.
- Verify WiFi disconnect/reconnect behavior (captive portal fallback to running).
- Test UI interaction via input buttons.
