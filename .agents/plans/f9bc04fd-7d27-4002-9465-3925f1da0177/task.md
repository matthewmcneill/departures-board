# Restore Simulator Rendering

- [x] Identify shadowing issue in `build_wasm.py`
- [ ] Reorder include flags in `build_wasm.py` to prioritize real U8g2
- [ ] Remove `U8G2_R0` and other mock-only defines from `tools/layoutsim/src/main.cpp`
- [ ] Clean up/Revert `test/mocks/U8g2lib.h` to its original minimal state
- [ ] Rebuild WASM and verify rendering in browser
