# 📍 Current State & Focus
The build timestamp automation is fully implemented and verified. The system now generates a second-level precise serial (`ByyyyMMddHHmmss-hash[+mod]`) on every build, ensuring perfect synchronization between the firmware (C++), the OLED loading screen, and the Web UI (System tab).

# 🎯 Next Immediate Actions
None. This task is 100% complete and verified. The user can continue with other UI or data provider refactorings in the next session.

# 🧠 Decisions & Designs
- **Header Generation**: Used a `pre` Python script in PlatformIO to generate a C++ header (`build_time.h`). This forces a re-compilation of `displayManager.cpp` and `systemManager.cpp` every time the serial changes, bypassing the stale cache issues.
- **Git Integration**: Integrated `git rev-parse --short HEAD` for commit hash tracking.
- **Dirty Detection**: Implemented `git diff --quiet` check to append `+mod` to the serial if uncommitted changes exist, providing honest versioning during rapid development cycles.
- **Web Parity**: Updated `portalBuilder.py` to regex-extract the `BUILD_TIME` from the C++ header during the asset build to ensure the web portal matches the firmware exactly.

# 🐛 Active Quirks, Bugs & Discoveries
- **Buffer Safety**: Audited all `snprintf` calls; the 15-27 character serial fits safely within the 32-byte display and communication buffers.

# 💻 Commands Reference
- `pio run -e esp32dev -t upload` : Standard build, flash, and serial update command.

# 🌿 Execution Environment
- Environment: `esp32dev`
- Git State: Clean (just committed at 0b614d3).
- Hardware: ESP32 Dev Module attached.
