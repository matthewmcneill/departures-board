---
name: "Broken Build Fix & Architecture Monolithic Strategy"
description: "Successfully resolved a fatal build failure caused by case-sensitivity mismatched `Logger.hpp` library paths and incorrect LDF recursion on nested `library.json` boundary files mapping to `displayMana..."
created: "2026-03-29"
status: "DONE"
commits: ['7718ab2']
---

# Summary
Successfully resolved a fatal build failure caused by case-sensitivity mismatched `Logger.hpp` library paths and incorrect LDF recursion on nested `library.json` boundary files mapping to `displayManager/fonts/`. Refactored the overall dependency management by stripping all nested `library.json` files from `modules/`, adopting a rigid Monolithic Build strategy governed strictly by `platformio.ini` as the single source of truth. Also updated `generate_layouts.py` to correctly integrate `systemBoard` dynamically generated JSON to CPP assets.

## Key Decisions
- **Monolithic Build Architecture**: Removed all sub-module `library.json` files to prevent PlatformIO's LDF from erroneously treating directories as strict module boundaries containing cyclic or missing references (specifically the fonts module).
- **Lowercase Header Alignment**: Mutated `lib/Logger` paths to `lib/logger` natively applying lowercase structures to match code inclusion directives (`#include <logger.hpp>`).
- **Configuration Path Rectification**: Corrected outdated compiler include paths inside `platformio.ini` pointing to non-existent `rssClient` and `dataWorker`, migrating them to `schedulerManager` and `dataManager` respectively.
- **SystemBoard Layout Automation**: Re-mapped Python layout preprocessor hooks to loop through the `systemBoard` JSON schemas.

## Technical Context
