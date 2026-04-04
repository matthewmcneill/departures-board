# Unify Mocking Architecture

> [!NOTE]
> **Audit Checklist**
> - [x] **House Style**: Standard `implementation_plan.md` formatting verified.
> - [x] **Architectural Standards**: Centralizing mock artifacts enforces Single Source of Truth (DRY) and prevents divergent dependencies.
> - [x] **UI Design**: N/A (Build system adjustment).
> - [x] **Resource Impact**: N/A (Test & Simulator configuration only).


This plan aims to unify the project's mocking architecture by establishing `test/mocks/` as the single source of truth for mocks, thereby eliminating duplicate and diverging files located in `tools/layoutsim/src/`. This will prevent silent failures and synchronization issues between the unit test logic and the layout simulator logic.

## User Review Required

> [!WARNING]
> Deleting the duplicate headers in `tools/layoutsim/src/` (such as `appContext.hpp` and `weatherClient.hpp`) and relying purely on `test/mocks/` will introduce compilation errors since the WASM simulator currently relies on distinct mock implementations.
> The layout simulator's `build_wasm.py` pipeline will be updated to point to the shared `.cpp` source files (e.g. `Arduino.cpp`, `appContext.cpp`, `Stubs.cpp`) natively found in `test/mocks/`. Any new build errors inside `layoutsim` will be rigorously addressed during the execution phase mapping them to the consolidated headers.

## Proposed Changes

---

### Duplicate Mocks Removal

These files currently shadow the global test mocks. Deleting them ensures that the compilation process defaults back to the single source of truth in `test/mocks/` (which `build_wasm.py` already includes through the `-I` flag).

#### [DELETE] [Arduino.h](file:///Users/mcneillm/Documents/Projects/departures-board/tools/layoutsim/src/Arduino.h)
#### [DELETE] [Arduino.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/tools/layoutsim/src/Arduino.cpp)
#### [DELETE] [FS.h](file:///Users/mcneillm/Documents/Projects/departures-board/tools/layoutsim/src/FS.h)
#### [DELETE] [LittleFS.h](file:///Users/mcneillm/Documents/Projects/departures-board/tools/layoutsim/src/LittleFS.h)
#### [DELETE] [Print.h](file:///Users/mcneillm/Documents/Projects/departures-board/tools/layoutsim/src/Print.h)
#### [DELETE] [SPI.h](file:///Users/mcneillm/Documents/Projects/departures-board/tools/layoutsim/src/SPI.h)
#### [DELETE] [WiFi.h](file:///Users/mcneillm/Documents/Projects/departures-board/tools/layoutsim/src/WiFi.h)
#### [DELETE] [Wire.h](file:///Users/mcneillm/Documents/Projects/departures-board/tools/layoutsim/src/Wire.h)
#### [DELETE] [appContext.hpp](file:///Users/mcneillm/Documents/Projects/departures-board/tools/layoutsim/src/appContext.hpp)
#### [DELETE] [weatherClient.hpp](file:///Users/mcneillm/Documents/Projects/departures-board/tools/layoutsim/src/weatherClient.hpp)

---

### Simulator Configuration

The python WASM build configuration will be updated to fetch the `.cpp` file implementations directly from `test/mocks/` since the respective `src/` duplicates will be removed.

#### [MODIFY] [build_wasm.py](file:///Users/mcneillm/Documents/Projects/departures-board/tools/layoutsim/scripts/build_wasm.py)
- Replace `os.path.join(SRC_DIR, "Arduino.cpp")` with `os.path.join(PROJECT_ROOT, "test", "mocks", "Arduino.cpp")`
- Explicitly add `os.path.join(PROJECT_ROOT, "test", "mocks", "appContext.cpp")` to `cpp_sources` list.
- Explicitly add `os.path.join(PROJECT_ROOT, "test", "mocks", "Stubs.cpp")` to the `cpp_sources` list if required.

## Open Questions

- Are there any specific simulator behaviors (like the Web UI layout logic bridging calls to JS) inherently tied to the old `appContext.hpp` that should be rewritten using a hook, or is adopting the unified `test/mocks/appContext.hpp` drop-in fully transparent?

## Verification Plan

### Automated Tests
- Run `python3 tools/layoutsim/scripts/build_wasm.py`.
- Ensure a clean compilation passes with no WASM undefined reference errors.
- Any resulting C++ errors resulting from missing functions/divergent signatures in `test/mocks/appContext.hpp` for the layout simulator files (`main.cpp`, `layoutParser.cpp`) will be fixed.
