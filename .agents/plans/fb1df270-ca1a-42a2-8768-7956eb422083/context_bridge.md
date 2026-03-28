# Context Bridge

## 📍 Current State & Focus
We have completed the requirements gathering and architectural design phase for adding dynamic multi-layout JSON support to all display boards (National Rail, TfL, Bus). The `implementation_plan.md` has been successfully audited by `house-style-documentation`, `architectural-refactoring`, and `embedded-systems` skills, and approved. We are currently blocked from execution due to a hardware lock held by another session.

## 🎯 Next Immediate Actions
1. Verify the hardware lock has been released (check `.agents/plans/lock.md`).
2. Run `/plan-start` to claim the hardware lock for this plan.
3. Proceed with executing the modifications defined in `implementation_plan.md` starting with `modules/configManager/configManager.hpp`.

## 🧠 Decisions & Designs
- Decided to evaluate target layout bindings recursively at lifecycle bound in `configure()` instead of `Board` constructors.
- Confirmed a memory-safe `strcmp` approach instead of using `String()` structures to switch target implementations, avoiding heap fragmentation on the ESP32.
- The `layout` string length in `BoardConfig` is capped at 32 characters to minimize static RAM `.bss` overhead (`<320 bytes` limit).

## 🐛 Active Quirks, Bugs & Discoveries
- N/A right now, execution has not begun. 
- *Attention*: Ensure `activeLayout` bounds are explicitly `delete`d prior to reassignment in `configure()` inside each controller, otherwise we will violently leak layout heap memory on consecutive reconfigurations.

## 💻 Commands Reference
- Build and Upload target firmware: `pio run -e esp32dev -t upload`
- Serial Monitor: `pio device monitor -e esp32dev`
- Testing local Web UI Simulator: `npx playwright test` inside `/test/web` directory (Phase 1 Web Testing).

## 🌿 Execution Environment
- The device hardware is actively in use by `c7f2e37a-a5c7-4952-9d93-d7d4efaf6374` (Scheduling System task).
