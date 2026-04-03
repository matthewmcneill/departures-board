# Context Bridge: Migration of Preprocessor Macros to Type-Safe Constants

## 📍 Current State & Focus
The refactoring of the firmware's status reporting and scheduling systems from legacy integer-based macros to type-safe `UpdateStatus` and `PriorityTier` enums is **100% complete**. Critical macro collisions with Arduino's `HIGH`/`LOW` have been resolved by adopting the `PRIO_` prefix for priority levels.

## 🎯 Next Immediate Actions
The plan is concluded. No immediate actions are required. The codebase is in a stable, buildable state.

## 🧠 Decisions & Designs
- **PriorityTier Prefixing**: Members renamed to `PRIO_LOW`, `PRIO_MEDIUM`, `PRIO_HIGH`, `PRIO_CRITICAL` to provide a globally unique namespace that avoids conflicts with generic `HIGH`/`LOW` hardware macros.
- **Strict Enum Return Codes**: All `iDataSource` derivatives now return `UpdateStatus` instead of `int`. Comparison logic in `DataManager` and `SystemManager` has been updated to handle these types using `static_cast<uint8_t>` for magnitude checks.
- **House Style Alignment**: All modified modules have updated module headers and documentation.

## 🐛 Active Quirks, Bugs & Discoveries
- **Macro Collisions**: Discovered that including `<Arduino.h>` or `<esp32-hal-gpio.h>` after defining `enum PriorityTier { LOW, HIGH }` results in compilation errors due to preprocessor macro substitution. Prefixing was the chosen solution to maintain compatibility.
- **String Conversion**: `UpdateStatus` requires explicit casting to `uint8_t` when being converted to a `String` for logging or web responses.

## 💻 Commands Reference
- Build: `pio run -e esp32dev`
- Test: `pio test -e unit_testing_host`

## 🌿 Execution Environment
- **Git Branch**: `refactor/technical-debt`
- **Hardware**: ESP32 Dev Module (Successfully built locally).
