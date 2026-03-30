# 📍 Current State & Focus
The **Layout Simulator (`displaysim`)** has been successfully stabilized and enhanced. It now features pixel-accurate rendering (linked to the real **U8G2** library) and a rearranged IDE with the **Logic Injection** panel on the far left. We have verified that live data from **BBC News**, **TfL Victoria**, and **National Rail** is correctly parsed and rendered on the virtual OLED.

The focus is now shifting to **Pillar 3**: Integrating this verified C++ parsing logic and JSON layout support into the main ESP32 firmware's `DisplayManager`.

# 🎯 Next Immediate Actions
1. **Firmware Integration**: Merge the `LayoutParser` and widget updates from the simulator branch into the main firmware build.
2. **Layout Deployment**: Upload the `layoutDefault.json` files for the National Rail and TfL boards to the ESP32 filesystem (`/data/`).
3. **Hardware Verification**: Flash the firmware and confirm that the physical OLED renders the same high-fidelity layout seen in the simulator.

# 🧠 Decisions & Designs
- **Build Priority**: Modifying `tools/layoutsim/scripts/build_wasm.py` to prioritize `U8G2_LIB_DIR` over `test/mocks` was critical to restoring rendering.
- **IDE Layout**: The UI rearrangement (Logic Injection on left) was finalized to improve usability after user feedback.
- **Mocking**: The `test/mocks/U8g2lib.hpp` file was preserved but bypassed for simulator builds to maintain compatibility with native unit tests.

# 🐛 Active Quirks, Bugs & Discoveries
- **Shadowing**: Caution is needed when adding search paths that might shadow standard library names (e.g., `U8g2lib.h`).
- **Path Bug**: Fixed a bug where `api/layouts` returned absolute local paths, which caused load failures in the browser.

# 💻 Commands Reference
- **WASM Build**: `python3 tools/layoutsim/scripts/build_wasm.py`
- **Dev Server**: `python3 tools/layoutsim/scripts/dev_server.py`
- **Data Synth**: `python3 tools/layoutsim/scripts/synth_live_data.py`

# 🌿 Execution Environment
- **Branch**: assumed main
- **Testing**: WASM Simulator verified at `http://localhost:8000/tools/layoutsim/web/index.html`.
