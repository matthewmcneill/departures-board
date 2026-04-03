# Defactor Macro Usage Task Plan

- [x] Investigate active `#define` macros across `modules/` and `src/`
- [x] Identify function signatures affected by return code macro replacements
- [x] Draft `implementation_plan.md` detailing module-by-module transition
- [x] Apply `house-style-documentation` guidelines to new definitions
- [x] Update `iDataSource.hpp` with `enum class` definitions
- [x] Propagate typed signatures to `tflDataSource`
- [x] Propagate typed signatures to `nationalRailDataSource`
- [x] Propagate typed signatures to `busDataSource`
- [x] Update system controllers (`systemManager`, `displayManager`)
- [x] Update `webHandlerManager.cpp`
- [x] Refactor `rssClient` (Done)
- [x] Refactor `weatherClient` (Updated PriorityTier)
- [x] Update `docs/` (Done AsyncDataRetrieval, SystemSpec)
- [ ] Replace UI and board configuration macros with `constexpr`
- [x] Run PlatformIO dry-run compilation (SUCCESS)
- [ ] Manual hardware verification
