# Context Bridge

## 📍 Current State & Focus
We are evaluating and fixing the `layoutsim` WebAssembly Simulator engine (`tools/layoutsim`) so it correctly builds against the modern V3 codebase architecture. We identified three specific compile issues preventing WASM linking: incorrect include path precedence shadowing the real U8g2 library with uncompilable headless mocks, a signature mismatch in `iDisplayBoard::updateData()` returning `int` instead of `UpdateStatus`, and missing forward declarations in `main.cpp`.

## 🎯 Next Immediate Actions
The arriving agent needs to execute the `implementation_plan.md` tasks exactly as detailed. This includes:
1. Modifying `tools/layoutsim/scripts/build_wasm.py` to move the `-I{test/mocks}` include directory AFTER the ESP32 and project core include flags.
2. Modifying `tools/layoutsim/src/displayManager.hpp` to return `UpdateStatus` in `updateData()`.
3. Inserting a forward declaration for `syncData` in `tools/layoutsim/src/main.cpp`.

## 🧠 Decisions & Designs
We decided to retain the inclusion of the test/mocks directory in the build script but relegate its priority. This is because the simulator still relies on certain structural mocks (like `Arduino.h` and `SystemState.hpp`), while needing the genuine library implementations for pixel UI rendering (`U8G2`) and message buffering (`MessagePool`).

## 🐛 Active Quirks, Bugs & Discoveries
The macOS case-insensitive file system previously allowed an include naming error (`MessagePool` vs `messagePool`), and the top-priority inclusion of headless mock code blocked the required graphics primitives (e.g. `drawLine`). Reording the include directories effectively mitigates these architectural quirks.

## 💻 Commands Reference
- WASM Simulator Build Command: `python3 tools/layoutsim/scripts/build_wasm.py`
- Background UI Web Server: `python3 tools/layoutsim/scripts/dev_server.py`

## 🌿 Execution Environment
- Environment contains active running display testing scripts, compiling via `emcc` for WASM simulations.
- We are not compiling firmware to the ESP32 physical hardware during this phase; WASM deployment is exclusively used for verifying the new widget UI layout.
