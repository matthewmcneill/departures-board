# Context Bridge

## 📍 Current State & Focus
We have exclusively operated in PLANNING mode to scope out the new Factory Pattern architecture for the DisplayManager (`departures-board` project). The `implementation_plan.md` has been drafted and fully reviewed sequentially against `house-style-documentation`, `architectural-refactoring`, and `embedded-systems` skills (`/ip-review` completed successfully). Work was deliberately paused before execution.

## 🎯 Next Immediate Actions
The arriving agent needs to immediately transition into `EXECUTION` mode and begin step 1 of the implementation plan: Refactoring `displayManager.hpp` to clear the `std::variant` memory pool.

## 🧠 Decisions & Designs
We've agreed to structurally move all system boards (e.g. `SplashBoard`) and user boards away from static member allocation/variant pools to Heap-based dynamic allocation explicitly instantiated using the new `BoardFactory`. Memory allocation will occur via `new` ONLY during boot and deep configuration states to prevent runtime heap fragmentation.

## 🐛 Active Quirks, Bugs & Discoveries
No active bugs. The previous `appContext` static design inherently tied everything to the compilation of the `displayManager`, meaning massive compilation cascades will happen during this refactor.

## 💻 Commands Reference
Building: `platformio run` or `/flash-test` via the slash commands workflow.

## 🌿 Execution Environment
Active Git Branch: Unspecified (likely feature branch). Hardware: ESP32 with OLED target. Local OS: Mac.
