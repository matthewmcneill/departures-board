# Task List for Logging Architecture Optimization

- [x] Planning Phase
  - [x] Review `lib/logger/logger.hpp` and `lib/logger/logger.cpp`.
  - [x] Review `src/departuresBoard.cpp` for `Logger::begin()`.
  - [x] Search for all invocations of `Logger::registerSecret()`.
  - [x] Draft an implementation plan covering:
    - Adding `LOG_REGISTER_SECRET` macro.
    - Wrapping `logger.cpp` contents in `#if CORE_DEBUG_LEVEL > 0`.
    - Updating direct calls to use the macro.
    - Adding conditional guards for `Serial.begin()`.
  - [x] Request user review of the implementation plan.
- [x] Execution Phase
  - [x] Update `lib/logger/logger.hpp`.
  - [x] Update `lib/logger/logger.cpp`.
  - [x] Update all callers of `registerSecret` (e.g., `configManager.cpp`).
  - [x] Update `src/departuresBoard.cpp` (or `logger.cpp`'s `begin` if there) conditionally.
- [x] Verification Phase
  - [x] Run compilation checks.
  - [x] Create walkthrough.md.
