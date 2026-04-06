# Context Bridge

**📍 Current State & Focus**: 
Research finished regarding upstream `gadec-uk` configuration updates structure. I have amended "The Epoch Translation Matrix" architecture inside `implementation_plan.md` based on user feedback. The translation algorithm will now map untagged legacy configs directly into our `LATEST` (e.g. `v3.0`/`v2.6+`) internal structures directly, rather than into a `v2.0` baseline, to prevent losing parallel upstream functionality additions. Currently in Planning mode, saved to disk ready for execution.

**🎯 Next Immediate Actions**: 
- Extract the legacy fallback mapping fields out of `modules/configManager/configManager.cpp`'s main parsing body (`ConfigManager::loadConfig()`).
- Implement the `detectConfigEpoch()` heuristic and the single-pass translation pipeline targeting the latest schema.

**🧠 Decisions & Designs**: 
- We decided on an Epoch isolation approach. We detect the structure of the incoming JSON (e.g., look for `"tube": true` vs `"mode": 1`) BEFORE attempting to parse it into our internal structs.
- If it is recognized as legacy upstream without a format `version`, we bypass our internal array of migrations and normalize it directly into our state-of-the-art memory mapping so that parallel upstream features aren't funneled out by older migration phases.

**🐛 Active Quirks, Bugs & Discoveries**: 
- Upstream `departures-board` does not include a `version` string in `config.json`. To handle OTA updates directly from upstream firmware to ours, we must sniff their JSON structure instead. Treating all non-versioned config objects blindly as `1.0f` risks crashing the `ConfigManager` JSON document limits over time.

**💻 Commands Reference**: 
- Using standard `pio run -e esp32s3nano -t erase` testing combinations for future application testing (no builds performed during this research session).

**🌿 Execution Environment**: 
- Branch environment intact. `modules/configManager/configManager.cpp` is clean. The primary artifacts (`implementation_plan.md`) exist locally.
- ⚠️ PORTABILITY RULE Acknowledged - No absolute paths used for repository files.
