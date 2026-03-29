# Context Bridge - Batch 4: Testing & Stability

## 📍 Current State & Focus
The project is entering the **Testing & Stability** phase (Batch 4). We have a solid foundation for the firmware, but the unit testing suite is currently a placeholder. We have just completed a deep architectural evaluation of the mocking strategy, identifying a powerful crossover between the **Native Test Rig** and the **WASM Layout Simulator**.

**Focus**: Unifying the mocking infrastructure to allow both automated logic testing and interactive visual stress-testing in the simulator.

## 🎯 Next Immediate Actions
1. **Unify Mocks**: Copy and adapt the existing mocks in `tools/layoutsim/src/` (Arduino, LittleFS, WiFi) into a central `test/mocks/` directory.
2. **Implement SystemState**: Create the shared `SystemState` mock to allow programmatic injection of WiFi, Weather, and OTA states.
3. **Build Audit**: Standardize `MAX_KEYS` and `MAX_BOARDS` in `departuresBoard.hpp` using `#ifndef` while preserving per-environment tuning in `platformio.ini`.
4. **WASM Bridge**: Expose the new `SystemState` setters to the `layoutsim` JS engine and add the "Logic Control Panel" to the simulator UI.

## 🧠 Decisions & Designs
- **Unified Mocking**: We are NOT creating a second set of mocks. We are using the mature `layoutsim` stubs as the foundation for the `unit_testing_host` to ensure behavioral parity.
- **Config Migration**: Legacy `turnOffOledInSleep` (v2.3) will migrate specifically to the `oledOff` property of `MODE_CLOCK` boards, matching the "sleep-only-on-screensaver" paradigm.
- **Data Snapshots**: We will implement a `dataHarvester.py` script to fetch real-world API snapshots for "Frozen Reality" testing in both the test rig and the simulator.

## 🐛 Active Quirks, Bugs & Discoveries
- **Inconsistent Constants**: `MAX_KEYS` is currently floating across different environments (4 vs 8) and is missing from the core header.
- **Simulator Stubs**: The `layoutsim` stubs are Emscripten-specific but can be trivially adapted for native Unity testing by wrapping them in standard C++ includes.

## 💻 Commands Reference
- Run Unit Tests: `pio test -e unit_testing_host`
- Build Firmware: `pio run -e esp32dev`
- Rebuild Layout Simulator: `python3 scripts/auto_build_wasm.py`

## 🌿 Execution Environment
- **Branch**: main (local)
- **Hardware**: ESP32 / ESP32-S3 (Mocked for current phase)
- **Testing**: Native C++ (Unity) + WASM (Layout Simulator)
