# Context Bridge

**📍 Current State & Focus**
The user has requested an implementation plan to remove the heavy `enableDebug` runtime toggling inside `dataManager`, shifting it instead to a lightweight compiler macro approach that maps to the existing hierarchy. The `implementation_plan.md` has been successfully drafted and is pending execution. The `/plan-save` workflow has been triggered to persist this setup for future execution.

**🎯 Next Immediate Actions**
1. Read the provided `implementation_plan.md` and execute its proposals.
2. Specifically modify `dataManager.cpp`, `dataManager.hpp`, and `appContext.cpp` to eliminate `enableDebug` entirely.

**🧠 Decisions & Designs**
- We decided against modifying the core SDK or global headers; instead, we are strictly removing the runtime parameter from `dataManager.hpp/cpp` and implementing a fast component-level `DATA_MANAGER_VERBOSE` macro that statically maps to `LOG_DEBUG` or a stripped-out NO-OP.

**🐛 Active Quirks, Bugs & Discoveries**
- The project enforces the `house-style-documentation` skill which explicitly forbids undocumented parameters or incorrectly labeled implementation plans. The current plan adheres to this.
- We are running on an ESP32 architecture using FreeRTOS.

**💻 Commands Reference**
- PlatformIO Build: `pio run`
- Device test/deploy: `/flash-test`

**🌿 Execution Environment**
- Hardware: ESP32 with OLED display.
- Compile framework: PlatformIO (Arduino Core wrapper).
