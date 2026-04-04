# Goal Description

The objective is to fix and stabilize the `layoutsim` WebAssembly Simulator engine (`tools/layoutsim`) so it builds using the latest versions of the C++ codebase. Recent codebase modifications (such as widget redefinitions and changes in interface signatures) caused the simulator build process to break.

Our analysis of the Emscripten (`emcc`) build logs reveals three primary issues:
1. **Redefinition of `MessagePool` and missing `U8G2` members:** The simulator build script `build_wasm.py` prioritizes `-I/test/mocks` before `-I.pio/libdeps/esp32dev/U8g2/src`. Since the mock folder has a stripped-down `U8g2lib.h` (used for Catch2 testing) and an obsolete `MessagePool.hpp`, the simulator attempts to build its pixel buffer using the mock logic and fails.
2. **Interface mismatch on `iDisplayBoard`:** The core interface `iDisplayBoard::updateData()` was recently refactored to return an `UpdateStatus` enum type. The simulator's `MockDisplayBoard` still incorrectly returns `int`, resulting in an override mismatch.
3. **Undeclared identifier `syncData`:** `syncData` is called multiple times early in `main.cpp` before its definition lower down. Emscripten’s strict C++17 compilation fails this missing forward-declaration.

## Proposed Changes

---

### Simulator Build Scripts
We will fix the library inclusion order so the WASM engine references the actual graphics library instead of the headless unit testing mocks.

#### [MODIFY] [build_wasm.py](file:///Users/mcneillm/Documents/Projects/departures-board/tools/layoutsim/scripts/build_wasm.py)
Move the `f"-I{os.path.join(PROJECT_ROOT, 'test', 'mocks')}"` directory inclusion flag to the **end** of `common_flags`. This ensures the simulator only uses `test/mocks` for fallback headers like `Arduino.h`, while fetching real library code (e.g. `U8G2`, `messagePool`) from their correct source locations.

---

### Simulator Mock Hardware Context
We will match the updated abstractions.

#### [MODIFY] [displayManager.hpp](file:///Users/mcneillm/Documents/Projects/departures-board/tools/layoutsim/src/displayManager.hpp)
Update the signature of `MockDisplayBoard::updateData()` from:
`int updateData() override { return 0; }`
to:
`UpdateStatus updateData() override { return UpdateStatus::SUCCESS; }`

---

### Simulator WASM Entry Point
We will fix compilation failures.

#### [MODIFY] [main.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/tools/layoutsim/src/main.cpp)
Add `void syncData();` block below the extern references, around line 57, to ensure forward-declaration before use in the exported debug stubs.

## Verification Plan

### Automated Tests
- Run `python3 tools/layoutsim/scripts/build_wasm.py` and ensure the simulator compiles successfully with 0 errors.
- Verify `dev_server.py` does not display any layout crashing errors during initialization.
