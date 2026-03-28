# Context Bridge

## 📍 Current State & Focus
We have completed the requirements gathering and architectural design phase for the new Scheduling System. The user has reviewed the visual calendar grid concept (Option B) vs the vertical list concept (Option A) and officially selected the **List Paradigm (Option A)** for the initial implementation due to its simplicity and mobile-first usability. Pencils were laid down immediately after finalizing the `implementation_plan.md` artifact. No code modifications have occurred yet.

## 🎯 Next Immediate Actions
1. Review the generated `task.md` locally.
2. Begin C++ backend implementation by creating the `schedulerManager.hpp` and `.cpp` files to fulfill the SRP and decoupled architecture.
3. Update `configManager.hpp` with the new data structures.

## 🧠 Decisions & Designs
- **One-to-One Mapping**: A single `ScheduleRule` struct points to exactly one board.
- **Implicit Carousel**: If multiple rules overlap in time, the `schedulerManager` returns all valid board IDs, and the existing `displayManager` handles the rotation via the new `carouselIntervalSecs` pacing.
- **Timeout Reset**: Pressing the physical button resets the `overrideTimestamp` on *every* press, extending the idle timeout limit.
- **Web UI Paradigm**: Option A (List Paradigm). A Mobile-First, 24-hour vertical layout using native HTML `<dialog>` elements instead of a heavy third-party calendar library or complex absolute-positioned DOM.

## 🐛 Active Quirks, Bugs & Discoveries
- Orphan Handling: If a board mapped to a saved schedule rule is deleted, the backend `schedulerManager` explicitly checks if `config.boards[index].complete == true` to avoid rendering blank screens. The UI must render a visible "Target Missing" warning for that rule slot.

## 💻 Commands Reference
- `npm run test` (in `test/web/`) for Playwright validation of the Web Portal changes.
- `pio run -e esp32dev -t upload` for hardware testing.

## 🌿 Execution Environment
- No physical hardware actions have occurred during this design phase. WASM play-testing can be used for UI validation.
