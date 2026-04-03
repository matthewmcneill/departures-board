Refactor the `dataManager` module to eliminate the runtime `enableDebug` flag. This anti-pattern consumes permanent heap memory and CPU cycles for logging that should be strictly compile-time. Instead of module-specific macros, we will implement a new global `LOG_VERBOSE` level in the core `Logger` library (Level 5). When `CORE_DEBUG_LEVEL` is set to 5 or higher, verbose logs will be compiled in; otherwise, they are stripped out as NO-OPs.

## User Review Required

> [!IMPORTANT]
> This change introduces a new global logging level `LOG_VERBOSE`. To see the "spammy" dataManager logs, the project must now be compiled with `-D CORE_DEBUG_LEVEL=5`.

> [!CAUTION]
> This change modifies the `init()` signature of the `dataManager` class. Any external modules (like `appContext`) that directly call `init(bool)` will need to be updated to `init()`.

## Proposed Changes

### Core Logger Library
#### [MODIFY] [logger.hpp](modules/systemManager/logger.hpp)
- Add `LOG_VERBOSE(sub, msg)` macro guarded by `#if CORE_DEBUG_LEVEL >= 5`.
- Add `static void _verbose(const char* category, const String& message)` and character pointer overload to the `Logger` class.

#### [MODIFY] [logger.cpp](modules/systemManager/logger.cpp)
- Implement `_verbose()` using the 🟣 (Purple Circle) icon for visual distinction.

### Data Manager Module
#### [MODIFY] [dataManager.hpp](modules/dataManager/dataManager.hpp)
- Remove `bool enableDebug = false` from the `init()` method signature.
- Remove the `bool debugEnabled` member variable from the class definition.

#### [MODIFY] [dataManager.cpp](modules/dataManager/dataManager.cpp)
- Remove `debugEnabled = enableDebug;` from the `init()` implementation.
- Replace all runtime `if (debugEnabled)` checks with direct invocations of the `LOG_VERBOSE` macro.

### Application Context
#### [MODIFY] [appContext.cpp](modules/appContext/appContext.cpp)
- Update the initialization call on line 161 from `networkManager.init(false);` to `networkManager.init();`.
- Clean up any adjacent stale comments referencing the old queue debug flag.

## Resource Impact Assessment

### Memory (RAM)
- **Heap**: Saving 1 byte per instance (singleton) by removing `debugEnabled`.
- **Stack**: Minor reduction in `init()` stack frame.
- **Static**: If `DATA_MANAGER_VERBOSE` is `0`, debug strings are not compiled in, saving several dozen bytes of Flash/RAM (depending on where the compiler places string literals).

### Flash (ROM)
- Reduction in code size as runtime checks are removed and debug logging is conditionally compiled.

### Power & Security
- No significant impact.

## Verification Plan

### Automated Tests
1. **Compilation Validation:** Run PlatformIO Build (`pio run`) locally to ensure the system links successfully and there are no signature mismatches or compiler warnings regarding `enableDebug`.

### Manual Verification
1. **Runtime Verification:** If the user permits via `run_command`, compile and flash the firmware using the `/flash-test` workflow. The system should boot normally and display data on the screen.
2. **Behavioral Trace:** Monitor the serial logs (via `/monitor`) across a standard boot cycle. Ensure the `dataManager` queue outputs are completely silent under normal DEBUG logging, validating the macro successfully strips out the noisy tracking strings without breaking execution flow.
