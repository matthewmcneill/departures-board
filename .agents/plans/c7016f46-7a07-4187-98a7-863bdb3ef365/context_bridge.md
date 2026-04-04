# Context Bridge: Intelligent Polling RDM

*   **📍 Current State & Focus**: The project currently relies on fixed 30-second interval polling for the National Rail RDM provider. The objective is to transition this to dynamic "intelligent" polling to conserve API boundaries and provide near real-time cancellation support. The plan has been reviewed and validated.
*   **🎯 Next Immediate Actions**: 
    1. Await the system lock release.
    2. Start implementation. Open `modules/displayManager/boards/nationalRailBoard/nrRDMDataProvider.cpp`.
    3. Implement datetime logic using `TimeManager` in `executeFetch()` for dynamic interval scaling.
*   **🧠 Decisions & Designs**:
    *   **Architecture**: Agreed to use `extern class appContext appContext;` inside `nrRDMDataProvider` instead of constructor injection for parity with existing hardware patterns.
    *   **Bounds**: Established a **15s absolute minimum** polling rate, and a **45s maximum** polling rate to enforce board reactivity.
*   **🐛 Active Quirks, Bugs & Discoveries**: 
    *   String time-parsing (`std` and `etd` as `HH:MM`) requires raw second-of-day math vs RTC structure values. Midnight rollovers (e.g. Train leaves `00:05` but time is `23:55`) require deliberate mathematical offsets `+- 86400`.
*   **💻 Commands Reference**:
    *   To view diagnostic debug outputs in real-time on hardware: `/monitor`
*   **🌿 Execution Environment**: ESP32 native hardware environment.
*   **⚠️ Portability Adherence**: All relative refs internally.
