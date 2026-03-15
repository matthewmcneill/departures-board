# Architectural Evaluation: AppContext State Machine

The suggestion to step back and view `appContext` holistically as a high-level state machine is an excellent architectural direction. Moving away from a monolithic, synchronous `setup()` block into a discrete, state-driven lifecycle will resolve the watchdog timeout fragility and provide a clearer user experience.

Here is a critical evaluation of your proposed states, followed by architectural recommendations for implementation.

## Evaluation of Proposed States

Your proposed states map beautifully to the user journey:
1. **`BOOTING`** (System initialized during setup(), displays Splash/Loading board)
2. **`CONFIG_WIFI_AP`** (No config / Failed to connect, shows Captive Portal) 
3. **`CONFIG_DISPLAYS`** (WiFi connected, but no boards configured, prompts for Portal)
4. **`RUN`** (Normal operation)
5. **Display-Level Errors** (Board handles its own data fetching/parsing errors)

### Pros of this approach:
- **Clean Separation of Concerns:** High-level app routing is handled by `appContext`, while lower-level retry logic remains inside `WiFiManager` and `DisplayBoards`.
- **Better UX:** Instead of the system hijacking the entire screen globally for a single board's error, the system robustly cycles through boards, letting broken boards report their specific issues (e.g., "API Key Invalid for Board 2") inline.
- **Robustness:** Booting becomes entirely non-blocking, eliminating watchdog panics.

### Constraints to Consider:
- `WiFiManager` already has its own internal state machine (`WIFI_INIT`, `WIFI_STA_CONNECTING`, `WIFI_AP_STARTING`, `WIFI_READY`). We must ensure `appContext` doesn't duplicate this logic.
- `systemManager` currently manages global refresh rates and HTTP polling. We need to ensure `systemManager` only polls data when `appContext` is in the `RUN` state.

---

## Recommended Architecture: Hierarchical State Machines (HSM)

Instead of a single massive state machine, the most elegant solution for this embedded device is a **Hierarchical State Machine**.

### 1. The Top-Level: `appContext` Application State
`appContext` acts as the overall director monitoring the ecosystem. It owns an `AppState` enum:
```cpp
enum class AppState {
    BOOTING,         // Initializing FS, Configs, Boot Splash
    WIFI_SETUP,      // WiFiManager is in AP mode. Show captive portal instructions.
    BOARD_SETUP,     // WiFi connected, but config.boards is empty. Show "Please configure" screen.
    RUNNING          // Normal operation.
};
```
**How it works in `tick()`:**
- `appContext::tick()` invokes `wifiManager.tick()`, `sysManager.tick()`, etc.
- It then computes its state reactively:
  - If `currentState == AppState::BOOTING`, verify if initialization is complete. Display the Splash or Loading board.
  - If `wifiManager.isAPMode()`, switch to `WIFI_SETUP` and tell `DisplayManager` to show the AP Info board.
  - Else if `!configManager.hasConfiguredBoards()`, switch to `BOARD_SETUP` and show the Web Portal Info board.
  - Else, switch to `RUNNING` and let `DisplayManager` rotation take over. Only in the `RUNNING` state should standard boards be requested.

### 2. The Network Level: `WiFiManager` (Internal State)
`WiFiManager` handles the gritty details of *how* we get online. It keeps its current state machine (`WIFI_INIT` $\rightarrow$ `WIFI_STA_CONNECTING` $\rightarrow$ `WIFI_READY` or fallback to `WIFI_AP_STARTING`). 
- `appContext` doesn't care *how* many times WiFi failed; it just asks `WiFiManager: Are you in AP mode?`

### 3. The Business Logic Level: `systemManager`
`systemManager` currently forces global error screens (`SYS_ERROR_TOKEN`, `SYS_ERROR_NO_DATA`) interrupting the whole system.
- **Action:** Refactor `systemManager::tick()`. It should *only* trigger `activeBoard->updateData()` if `appContext` is in the `RUNNING` state.
- **Action:** Remove global error board forcing from `systemManager`.

### 4. The Presentation Level: `iDisplayBoard` (Inline Error Reporting)
As you brilliantly suggested, a misconfigured board should own its failure.
- Add an `errorState` to `iDisplayBoard` (e.g., `OK`, `AUTH_FAILED`, `FETCH_TIMEOUT`).
- When `systemManager` calls `activeBoard->updateData()`, the board internally flags itself if it fails.
- When `DisplayManager` asks the board to `draw()`, the board checks its own `errorState`. If it failed, it renders an error message (e.g., "National Rail: Invalid API Key. Fix in portal") *during its designated time slot*.

---

## Contending Alternative: Event-Driven (Pub/Sub) Architecture
Instead of `appContext` checking states every `tick()`, managers could emit events 
(e.g., `wifiManager.emit(WIFI_ENTERED_AP)`, `configManager.emit(BOARDS_UPDATED)`).
- **Pros:** Highly decoupled.
- **Cons:** In C++ for ESP32, building a robust event bus adds overhead and complexity, making the code harder to trace. The reactive polling in `appContext::tick()` (HSM) is far simpler and perfectly suited for a fast 60Hz loop.

## Summary of Execution Steps
1. **Define `AppState`** enum in `appContext.hpp`.
2. **Refactor `appContext::tick()`** to determine `AppState` and force the appropriate UI `SystemBoard` (`SYS_BOOT_SPLASH`, `SYS_WIFI_AP`, `SYS_BOARD_SETUP`) when not in `RUNNING`.
3. **Refactor `systemManager`** to stop handling global HTTP failures with fullscreen takeovers, and only poll data when `appContext` is `RUNNING`.
4. **Refactor `iDisplayBoard`** inheritors (e.g., `NationalRailBoard`) to buffer their own HTTP status codes and render inline error UI if `updateData()` failed.
