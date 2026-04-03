# Context Bridge: Firmware Logging & Memory Architecture Refactor

- **📍 Current State & Focus**: Phase 4 of the memory architecture refactor is complete. The system's high-frequency logging and XML/JSON network polling now operate using zero transient `String` object instances, effectively eliminating long-term dynamic heap fragmentation. The `Logger`, `weatherClient`, `busDataSource`, and `rssClient` have all been refactored to use static or stack-allocated `char` buffers (`snprintf`) and in-place redaction.
- **🎯 Next Immediate Actions**: Assess the `.agents/todo_list.md` to identify the next high-priority task, potentially focusing on the remaining items concerning the config constraints or further encapsulation.
- **🧠 Decisions & Designs**: 
  - Standardized on phase 1's `printf`-style logging macros (`LOG_INFOf`, `LOG_ERRORf`).
  - Switched XML tag tracking in `rssClient` from multiple String allocations to a single fixed `char tagPath[64]` buffer.
  - Implemented `redactInPlace` in `Logger` using `strstr` to mask sensitive keys within stack buffers without allocating new `String` instances.
- **🐛 Active Quirks, Bugs & Discoveries**: PlatformIO IDE caching occasionally throws linting errors (`Arduino.h` file not found), but these do not impact the physical `pio run` build process.
- **💻 Commands Reference**:
  - Build firmware: `pio run -e esp32dev`
  - Host tests: `pio test -e unit_testing_host`
- **🌿 Execution Environment**: Active git branch contains the committed String refactoring changes. No physical hardware is strictly required for this stage, all host-level tests pass.
