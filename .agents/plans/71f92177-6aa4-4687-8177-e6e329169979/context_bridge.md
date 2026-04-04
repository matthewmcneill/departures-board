# Context Bridge

## 📍 Current State & Focus
The project has duplicate mock headers. We are standardizing the mock architectures by establishing `test/mocks/` as the single source of truth for both `test/` (native testing pipeline) and `tools/layoutsim/` (the WASM layout emulator). The current focus is finalizing the unification plan. We have created an `implementation_plan.md` which lists 10 duplicate headers currently in `tools/layoutsim/src/` to be deleted and points out the necessary source corrections for the Emscripten config `tools/layoutsim/scripts/build_wasm.py`.

## 🎯 Next Immediate Actions
1. Receive approval to execute the plan or load it via `/plan-queue`.
2. Delete the 10 headers outlined in `implementation_plan.md` from `tools/layoutsim/src/`.
3. Modify `tools/layoutsim/scripts/build_wasm.py` to point its explicit `.cpp` targets to `test/mocks/` instead of `tools/layoutsim/src/`.
4. Run `python3 tools/layoutsim/scripts/build_wasm.py` and resolve any missing definitions or compilation errors that arise from adopting the strict globals.

## 🧠 Decisions & Designs
- **Single Source of Truth**: We will no longer support shadow mocks inside `tools/layoutsim/src/`. All simulation bindings and headers must utilize the main project's `test/mocks/` module structure to ensure synchronous testing layers and prevent silent injection bugs in WASM.

## 🐛 Active Quirks, Bugs & Discoveries
- The simulator builds with Emscripten; swapping the definitions to `test/mocks/` means any functions missing implementations in `test/mocks/appContext.cpp` that `tools/layoutsim/src` originally contained will trigger linking errors. Expect WASM compilation errors that need fixing during the execution step.

## 💻 Commands Reference
- WASM Simulator build: `python3 tools/layoutsim/scripts/build_wasm.py`
- Layout Simulator HTTP Server: `python3 tools/layoutsim/scripts/dev_server.py`
- Terminal Tests: `npm test` inside `test/web`

## 🌿 Execution Environment
- Branch: `main`
- Modality: Target focus is WASM / C++ emulation via Emscripten; native flashing locks aren't exclusively required for this build.
