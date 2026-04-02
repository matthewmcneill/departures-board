# Context Bridge

**📍 Current State & Focus**
The implementation plan for replacing legacy `#define` macros with `constexpr` variables and strongly-typed `enum class` definitions has been completely drafted. It has been audited by the `house-style-documentation` and `embedded-systems` skills to verify zero adverse resource impacts on flash/RAM. The user has halted the execution before the coding phase to persist the plan. The focus of the next session will be executing this plan.

**🎯 Next Immediate Actions**
1. Read `implementation_plan.md` to understand the full suite of proposed code modifications.
2. In `iDataSource.hpp`, replace `UPD_*` and `TIER_*` macros with typed `enum class UpdateStatus` and `enum class PriorityTier` instances respectively.
3. Propagate these modified typed return signatures to `nationalRailDataSource.cpp`, `busDataSource.cpp`, `tflDataSource.cpp`, `weatherClient.cpp`, and downstream controllers (`systemManager.cpp`, `webHandlerManager.cpp`, `displayManager.cpp`).
4. Replace dimensions inside `departuresBoard.hpp` and board UI `.cpp` files with `constexpr size_t` bindings.

**🧠 Decisions & Designs**
- Agreed to encapsulate the `UpdateStatus` and `PriorityTier` enum classes within `iDataSource.hpp` rather than an independent global file, optimizing for component locality.
- Dimensional limits like `#define DIMMED_BRIGHTNESS 15` will become strictly scoped `constexpr` inside their utilizing `.cpp` files.

**🐛 Active Quirks, Bugs & Discoveries**
- Extreme care must be taken refactoring `int taskStatus` to `volatile UpdateStatus taskStatus` across various async networking loops, assuring the type mappings propagate accurately across execution spaces.

**💻 Commands Reference**
- Use `pio run -e esp32-s3-wroom-1-n16r8` directly into your terminal periodically to verify strict typing constraints execute at compile time successfully post-refactoring.

**🌿 Execution Environment**
- Environment: Local `departures-board` repository. Firmwares are actively validating on an ESP32 target context.
