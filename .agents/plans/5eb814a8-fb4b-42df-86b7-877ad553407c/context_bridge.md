# Context Bridge

**📍 Current State & Focus**
We just drafted an `implementation_plan.md` to refactor the boot sequence progress bar. We identified a fundamental deadlock bug in `systemManager::tick()` causing the `firstLoad` flag to drop instantly. The user requested a `/plan-save` manually before approving or executing the implementation plan.

**🎯 Next Immediate Actions**
The next agent needs to execute the `implementation_plan.md` tasks across `appContext` and `systemManager`.

**🧠 Decisions & Designs**
- Mapped synchronous boot payload across 0-30%.
- Mapped Clock sync down to 30-60%.
- Extended `systemManager::tick()` background sweep logic: conditionally allow data fetching when `(appState == AppState::RUNNING || firstLoad)` to resolve the load screen drop.
- Moved `firstLoad = false` to trigger *only* natively when a board fetch is actually finalized (`!noDataLoaded`), except falling back to instantly dropping if `config.boardCount == 0`.
- Ensured `appContext` naturally falls into `WIFI_SETUP` or `BOARD_SETUP` modes before locking on `firstLoad`.

**🐛 Active Quirks, Bugs & Discoveries**
- The `LoadingBoard` in `appContext` naturally loops the progress numbers if you provide a looping hook (like during system clock sync). We shifted this to visually represent 30% to 60%.
- `onBootProgress` callbacks were heavily fragmented across `systemManager`.

**💻 Commands Reference**
- Use `/flash-test` to test on hardware.
- Use `/review-ip` to automatically apply house style to any edited implementation plans.

**🌿 Execution Environment**
- ESP32 hardware development, targeting `.agents/plans/5eb814a8-fb4b-42df-86b7-877ad553407c`
