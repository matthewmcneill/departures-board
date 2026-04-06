# Context Bridge

**📍 Current State & Focus**: 
We are finalizing the Upstream Configuration Migration architecture prior to code implementation. I have fully authored and consolidated both Strategy A (Epoch Translation) and Strategy B (Non-Destructive Version-Locked Suffixing) into the `implementation_plan.md`. The plan now explicitly relies on dynamic evaluation driven by `CONFIG_VERSION_MAJOR` and `CONFIG_VERSION_MINOR` constants in `configManager` instead of hardcoded strings. Execution is prepared but paused.

**🎯 Next Immediate Actions**: 
- Boot up via `/plan-start` to claim the hardware build locks.
- Create `modules/configManager/gadecMigration.hpp` and `.cpp` according to the exact interfaces laid out in `implementation_plan.md`.
- Refactor `ConfigManager::loadConfig()` to iterate gracefully back through available `config_X_Y.json` files via `LittleFS.openDir("/")` before translating them.

**🧠 Decisions & Designs**: 
- We aggressively embraced the **Non-Destructive Version-Locked Suffixing** architecture. 
- Original upstream configurations (`config.json`) will be strictly read-only within our firmware. Any mutations originated within v3.0 logic will be structurally re-mapped and saved explicitly as new version-locked files (e.g., `config_2_6.json`), ensuring `Gadec-uk` baseline state is unbreakable if the user downgrades/reverts their firmware.
- Hardcoded `"/config_2_6.json"` file strings were explicitly vetoed in favor of dynamically constructed C++ macro pairs (`CONFIG_VERSION_MAJOR`).

**🐛 Active Quirks, Bugs & Discoveries**: 
- Active Design Conflict: What do we do about `apikeys.json` destruction? When rewriting `apikeys.bin`, we explicitly delete `apikeys.json`. Reverting firmware to upstream will render API connectivity broken. We must patch this in implementation.

**💻 Commands Reference**: 
- Standard PlatformIO building paths apply: e.g. `pio run -e esp32s3nano -t erase` or `/flash-test`.

**🌿 Execution Environment**: 
- Planning block purely text-based.
- PORTABILITY RULE adhered to heavily (all internal `implementation_plan.md` paths reference relative module directories).
