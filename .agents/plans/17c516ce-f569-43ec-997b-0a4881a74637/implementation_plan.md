# Refactoring dataManager Logging Framework

Refactor the `dataManager` module to eliminate the runtime `enableDebug` flag. This anti-pattern consumes permanent heap memory and CPU cycles for logging that should be strictly compile-time. The boolean flag will be replaced by a module-level preprocessor macro (`DATA_MANAGER_VERBOSE`) that maps to standard logging utilities, improving memory efficiency and encapsulation.

## Proposed Changes

### Data Manager Module
#### [MODIFY] [dataManager.hpp](file:///Users/mattmcneill/Personal/Projects/departures-board/modules/dataManager/dataManager.hpp)
- Remove `bool enableDebug = false` from the `init()` method signature.
- Remove the `bool debugEnabled` member variable from the class definition.
- Verify that Doxygen comments for `init()` strictly align with the `house-style-docs` standards and accurately reflect the simplified signature.

#### [MODIFY] [dataManager.cpp](file:///Users/mattmcneill/Personal/Projects/departures-board/modules/dataManager/dataManager.cpp)
- Add a new block at the top of the file explicitly defining `DATA_MANAGER_VERBOSE` as `0` by default.
- Add preprocessor directives to define `LOG_DM_VERBOSE` based on `DATA_MANAGER_VERBOSE`. When enabled, it maps to `LOG_DEBUG`; when disabled, it strips out the trace strings via `do {} while(0)`.
- Replace all runtime `if (debugEnabled)` checks with direct invocations of the `LOG_DM_VERBOSE` macro.
- Standardize the `init()` implementation signature.

### Application Context
#### [MODIFY] [appContext.cpp](file:///Users/mattmcneill/Personal/Projects/departures-board/modules/appContext/appContext.cpp)
- Update the initialization call on line 147 from `networkManager.init(false);` to `networkManager.init();`.
- Clean up any adjacent stale comments referencing the old queue debug flag.

## Verification Plan

### Automated Tests
1. **Compilation Validation:** Run PlatformIO Build (`pio run`) locally to ensure the system links successfully and there are no signature mismatches or compiler warnings regarding `enableDebug`.

### Manual Verification
1. **Runtime Verification:** If the user permits via `run_command`, compile and flash the firmware using the `/flash-test` workflow. The system should boot normally and display data on the screen.
2. **Behavioral Trace:** Monitor the serial logs (via `/monitor`) across a standard boot cycle. Ensure the `dataManager` queue outputs are completely silent under normal DEBUG logging, validating the macro successfully strips out the noisy tracking strings without breaking execution flow.
