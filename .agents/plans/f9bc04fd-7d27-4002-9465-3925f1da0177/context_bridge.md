# 📍 Current State & Focus
The **Layout Simulator (`displaysim`)** has been stabilized and verified. Rendering is functional with the real **U8g2** library, and the **Logic Injection** panel has been rearranged to the far left. The project has been branched to `feature/simulator-fix-v3` and pushed to origin.

The focus is now on **stateful mocking** for the Logic Injection panel (WiFi, Weather, OTA) to ensure the C++ widgets react to UI inputs.

# 🎯 Next Immediate Actions
1. **Stateful WiFi Mock**: Modify `test/mocks/WiFi.h` to allow setting status and RSSI from the WASM glue code.
2. **Weather Logic Wiring**: Connect the UI's weather/temp selectors to the boards' `WeatherStatus` objects.
3. **OTA Progress Visualization**: Wire the OTA slider to the `progressBarWidget` in the active layout.

# 🧠 Decisions & Designs
- **Branch Strategy**: Moved code to `feature/simulator-fix-v3` for modular development of the v3.0 logic injection features.
- **Rendering Restoration**: Prioritized real U8g2 includes in `build_wasm.py` to fix the black screen issue.

# 🐛 Active Quirks, Bugs & Discoveries
- **Shadowing**: The `test/mocks` directory can shadow real libraries if not carefully ordered in the include flags.

# 💻 Commands Reference
- **WASM Build**: `python3 tools/layoutsim/scripts/build_wasm.py`
- **Dev Server**: `python3 tools/layoutsim/scripts/dev_server.py`

# 🌿 Execution Environment
- **Branch**: `feature/simulator-fix-v3`
- **Hardware**: WASM Simulator active at `http://localhost:8000`.
