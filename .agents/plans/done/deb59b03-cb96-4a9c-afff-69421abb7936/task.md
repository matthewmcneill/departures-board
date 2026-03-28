# Tasks

- [x] Draft Implementation Plan (Display Logging)
- [ ] Extend `iDisplayBoard` with `getBoardName()`
- [ ] Implement `getBoardName()` across all 11 board types
- [x] Enhance `showBoard` footprint with `reason` string parameter
- [x] Add explicit logs to `appContext` Observer lambdas
- [x] Verify functionality (Compilation & Serial Output)

## Phase 4: UI Animation & Screen Refactoring
- [x] Migrate `resetState` to `resumeDisplays` across `appContext` orchestrator.
- [x] Expand vertical bounds of `progressBarWidget` to `24px` to eliminate text collision.
- [x] Display `Version: 3.0` and `Build: <time>` anchors cleanly on `LoadingBoard`.
- [x] Synchronize OLED `sendBuffer` to ~60FPS hardware throttle.
- [x] Construct 1sec/2sec/1sec cinematic screen fade sequence for `SplashBoard`.
