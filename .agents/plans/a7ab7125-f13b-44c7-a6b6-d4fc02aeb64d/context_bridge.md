# 📍 Current State & Focus
Refactoring the ESP32 firmware's networking modules (`busDataSource`, `weatherClient`) and `systemManager` to eliminate `String` objects, replacing them with C-style bounded `char` arrays (capped at 64 bytes). This addresses potential heap fragmentation for 24/7 uptime stability.

# 🎯 Next Immediate Actions
1. **Modify `busDataSource.hpp`**: Replace `currentKey` and `currentObject` with `char[64]` and update the string-resolution logic.
2. **Modify `weatherClient.hpp`**: Similar refactoring for keys and OWM API fields.
3. **Refactor `systemManager::getBuildTime()`**: Change return type to `const char*` using a static buffer.
4. **Update Scraper Logic**: Transition the `busDataSource` HTML scraper from `readStringUntil` to bounded buffer reads if possible.

# 🧠 Decisions & Designs
- **Buffer Safety**: 64-byte caps adopted universally for JSON keys/objects to exceed known maximums (e.g., `BUS_MAX_LOCATION = 45`) with minimal memory trade-off on ESP32.
- **Interfacing**: Acknowledge that `JsonListener` uses `String` in its virtual signatures; we will accept these but convert to `char[]` immediately to avoid persistent heap storage.

# 🐛 Active Quirks, Bugs & Discoveries
- `busDataSource.cpp`'s HTML scraper is a high-allocation point using `readStringUntil('\n')`. This will require a more surgical implementation to maintain performance while removing `String`.

# 💻 Commands Reference
- Build: `pio run -e esp32dev`
- Test: `pio test -e unit_testing_host`

# 🌿 Execution Environment
- **Branch**: main
- **Hardware**: ESP32 Attached
