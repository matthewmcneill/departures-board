# Context Bridge

**📍 Current State & Focus**
We have successfully drafted an Implementation Plan to optimize the memory footprint of the logging architecture on the ESP32. Specifically, we've outlined macro-wrapping `Logger::registerSecret` and `#if` protecting the entire `logger.cpp` implementation body to eliminate `std::vector<String>` memory allocation during release builds (`CORE_DEBUG_LEVEL == 0`). We were awaiting user approval on the plan before proceeding to execution.

**🎯 Next Immediate Actions**
1. Check with the user if the proposed implementation plan is approved.
2. If approved, switch to EXECUTION mode and begin applying changes to `logger.hpp`, `logger.cpp`, `configManager.cpp`, and `departuresBoard.cpp` as outlined in the `implementation_plan.md` artifact.

**🧠 Decisions & Designs**
- Create `#define LOG_REGISTER_SECRET(secret)` macro.
- Create `#define LOG_BEGIN(...)` macro.
- Everything inside `logger.cpp` except `#include` will be wrapped in `#if CORE_DEBUG_LEVEL > 0`.
- All methods must be maintained with strict house-style documentation.

**🐛 Active Quirks, Bugs & Discoveries**
- Discovered that passing the compiler flag to disable logging didn't strip out heap string manipulation due to `configManager` invoking the method manually in ways that evaded the compile-time checks, hence the zero-overhead initiative.

**💻 Commands Reference**
- Build firmware: `pio run -e esp32dev`

**🌿 Execution Environment**
- PlatformIO ESP32 (env: esp32dev or esp32s3nano)
- Currently on active branch.
