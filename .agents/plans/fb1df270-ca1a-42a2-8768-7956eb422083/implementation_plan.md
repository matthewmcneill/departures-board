[x] Reviewed by house-style-documentation - passed
[x] Reviewed by architectural-refactoring - passed
[x] Reviewed by embedded-systems - passed, resource impact added

# Goal
This document proposes the architecture for supporting multiple JSON and C++ layouts per display type, and outlines the engineering steps to build it.

## Resource Impact Assessment

> [!WARNING]
> The addition of dynamic layout binding has the following resource implications for the ESP32:
> - **Memory (RAM/Heap):** Expanding `BoardConfig` by adding a 32-character buffer increases the global allocation by `32 bytes * MAX_BOARDS`. Since `MAX_BOARDS` is typically low (e.g., 5-10), the total RAM impact is `<320 bytes`, which is negligible. Using `strcmp` rather than `String` objects mitigates heap fragmentation risk. The dynamic layout initialization explicitly `delete`s the old layout before re-allocating a new pointer, ensuring we do not leak layout memory.
> - **Flash/ROM:** Adding `strcmp` evaluations and linking layout sub-classes (`layoutReplica.cpp` etc.) adds a minor marginal increase (est. < 2 KB) to the `.text` segment. 
> - **Security/Power:** No external threat vectors are exposed by shifting layout mapping to `config.json`, and dynamic switching executes synchronously on the board tick thread, creating no measurable power draw penalty.

## a) What is the default if nothing is configured?
By default, if nothing is defined, the system will use a `"default"` reference. This guarantees backward compatibility with existing configurations dynamically generated or persisted to `config.json`. 
In the firmware, if a board receives an empty or `"default"` string during configuration, it will instantiate its primary layout class (e.g., `new layoutNrDefault(context)`), and fall back appropriately without causing a crash or UI disruption.

## b) How do we store it in the configuration?
We will extend the core C++ application configuration structures and its JSON parser mappings:
1. **Extend `BoardConfig`:** In `modules/configManager/configManager.hpp`, we will add a new field to `BoardConfig`:
   ```cpp
   char layout[32] = ""; // Target layout ID, defaults to empty (i.e., 'default')
   ```
2. **Persistence Logic:** During `ConfigManager::loadConfig()` and `ConfigManager::save()`, we will serialize/deserialize the `"layout"` dictionary key alongside the existing settings like `.id`, `.name`, and `.timeOffset`.
3. **Firmware Upgrades:** If a user upgrading from v2.4 loads older JSON, the `layout` key won't exist which drops back to the safe `""` blank value.

## c) How do we change it in the portal and give the user the choice of display in run-time?

**1. Web Portal Modifications (The Setup Choice):**
* Locate the board editing interface in the `web/` UI.
* Insert a new "Display Layout" dropdown menu inside the `Board Configuration Modal`.
* The available `<option>`s will be contextually bound to the board type (e.g. If editing a National Rail Board, show "Default Layout" and "Replica Layout". If editing TfL, show "Standard Layout").
* When the user presses "Save," the serialized JSON payload sent to the API contains `"layout": "replica"`.

> [!NOTE]
> **Proposed UI Update Mockup:**
> ```text
> ┌───────────────────────────────────────────────┐
> │ Edit Board: London Waterloo                   │
> ├───────────────────────────────────────────────┤
> │     Display Layout:                           │
> │     ┌───────────────────────────────────────┐ │
> │     │ Standard Default                      ▼ │
> │     ├───────────────────────────────────────┤ │
> │     │ Standard Default                      │ │
> │     │ Platform Replica                      │ │
> │     └───────────────────────────────────────┘ │
> └───────────────────────────────────────────────┘
> ```

**2. Firmware Run-Time Binding:**
Currently, `NationalRailBoard` (and other boards) hard-codes layout initialization directly in its Constructor:
```cpp
NationalRailBoard::NationalRailBoard(appContext* contextPtr) {
    activeLayout = new layoutNrDefault(context);
}
```
* **Delay Allocation:** We will move layout allocation to the `configure(const BoardConfig& config)` lifecycle hook across all supported boards (`NationalRailBoard`, `BusBoard`, `TflBoard`). We will substitute the layout dynamically using memory-safe `strcmp` instead of `String` objects to minimize heap fragmentation:
```cpp
void NationalRailBoard::configure(const BoardConfig& config) {
    // Purge existing layout pointer before re-allocating
    if (activeLayout) { delete activeLayout; activeLayout = nullptr; }

    // Use a factory block and strcmp to allocate the right class safely
    if (strcmp(config.layout, "replica") == 0) {
        activeLayout = new layoutReplica(context);
    } else {
        // Fallback or "default"
        activeLayout = new layoutNrDefault(context); 
    }
    
    // ... complete the rest of `configure()` 
}
```
*(This pattern will be mirrored for `TflBoard` and `BusBoard`, ensuring they destroy their previous `activeLayout` and allocate the requested class variants before proceeding with initialization)*.

* **Simulator Preview (WASM):** The WASM Simulator's React wrapper needs to observe the `layout` drop-down selection and swap the path loaded in the `BoardSimulator` component (fetching `/layouts/layoutReplica.json` rather than the `layoutDefault.json`).

## Proposed Changes
### Backend & Config
#### [MODIFY] [configManager.hpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/configManager/configManager.hpp)
Add `char layout[32] = ""` to `BoardConfig`.
#### [MODIFY] [configManager.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/configManager/configManager.cpp)
Update `save()` and `loadConfig()` JSON mapping methods to serialize and deserialize `doc["boards"][i]["layout"]`.
### Board Controllers
#### [MODIFY] [nationalRailBoard.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/boards/nationalRailBoard/nationalRailBoard.cpp)
Move `activeLayout` initialization out of constructor into `configure()`, checking `strcmp(config.layout, "replica")`.
#### [MODIFY] [busBoard.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/boards/busBoard/busBoard.cpp)
Move `activeLayout` initialization to `configure()`.
#### [MODIFY] [tflBoard.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/boards/tflBoard/tflBoard.cpp)
Move `activeLayout` initialization to `configure()`.

## Verification Plan
1. Configure a board with default payload in `config.json` manually to ensure it boots to default correctly without crashing.
2. Edit `config.json` manually so layout is set to `"replica"`, ensure C++ logic successfully triggers construction of `layoutReplica`.
3. *(Web Portal modifications would be performed in a subsequent web-specific workflow since this plan addresses C++/Backend wiring).*

## User Review Required

> [!IMPORTANT]
> - Do you agree with scoping the `layout` target field to 32 chars?
> - Is the layout only applicable to **National Rail** right now, or do you have variants designed for TfL or Bus ready to be exposed to the UI soon?
> - Should I proceed with making these edits to the `configManager` and the C++ backend right now?
