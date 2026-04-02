[x] Reviewed by house-style-documentation - passed
[x] Reviewed by architectural-refactoring - passed
[x] Reviewed by embedded-systems - passed

# Zero-Overhead Logging Architecture Refactor

## Goal Description
Optimize the logging architecture in the `departures-board` project to achieve true zero-overhead compilation on the ESP32 when debugging is disabled (`CORE_DEBUG_LEVEL == 0`). Specifically, we will resolve memory leaks caused by `Logger::registerSecret()` allocating memory for string redaction even when the logger is entirely disabled by wrapping it in a macro, and we will place the `logger.cpp` definitions under compile-time conditionals. 

## Resource Impact Assessment
### Memory (Flash/RAM/Stack/Heap)
- **Flash/ROM**: Reduction in binary size as the entire `Logger` implementation and its supporting data structures (like `std::vector<String>`) are compiled out when `CORE_DEBUG_LEVEL == 0`.
- **RAM/Heap**: Significant savings on the heap. Previously, `Logger::registerSecret` would allocate `String` objects into a global vector regardless of whether logging was enabled. This plan ensures that when logging is disabled, no allocations occur, and the vector itself is not instantiated.
- **Stack**: No significant change to stack usage.

### Power
- **Power Consumption**: Negligible direct impact on power, though reduced CPU cycles for string manipulation during boot (secret registration) may marginally improve boot time and energy consumption.

### Security
- **Security Posture**: No negative impact. String redaction functionality is maintained for debug builds. Secret registration remains available but optimized.

## User Review Required
No blocking decisions required. The structure ensures `Logger::secrets` vector definition and `Serial.begin` bindings compile out completely.

## Proposed Changes

### Logger Module
#### [MODIFY] [logger.hpp](file:///Users/mattmcneill/Personal/Projects/departures-board/lib/logger/logger.hpp)
- Define a preprocessing macro `#define LOG_REGISTER_SECRET(secret)` that behaves as follows:
  - If `CORE_DEBUG_LEVEL > 0`, expands to `Logger::registerSecret(secret)`.
  - If `CORE_DEBUG_LEVEL == 0`, expands to an empty no-op.
- Define an analogous preprocessing macro `#define LOG_BEGIN(...)` mapping to `Logger::begin(...)` for consistency and conditional execution.
- Maintain Doxygen comments according to the house-style documentation.

#### [MODIFY] [logger.cpp](file:///Users/mattmcneill/Personal/Projects/departures-board/lib/logger/logger.cpp)
- Conditionally compile the internal data attributes and function bodies by placing them all within a parent `#if CORE_DEBUG_LEVEL > 0` block immediately after `#include <logger.hpp>`. 
- Doing this allows the `std::vector<String> secrets` pooling system to compile out and disappear from RAM when debug logging is zero.

### ConfigManager Module
#### [MODIFY] [configManager.cpp](file:///Users/mattmcneill/Personal/Projects/departures-board/modules/configManager/configManager.cpp)
- Change line 142 (approx): replace `Logger::registerSecret(config.keys[i].token);` with `LOG_REGISTER_SECRET(config.keys[i].token);`.

### Application Bootloader
#### [MODIFY] [departuresBoard.cpp](file:///Users/mattmcneill/Personal/Projects/departures-board/src/departuresBoard.cpp)
- Change line 120 (approx) from `Logger::begin();` to `LOG_BEGIN();` via the new macro to prevent `Serial.begin()` initialization when telemetry is off.
- This ensures zero overhead on setup.

## Verification Plan
### Automated Tests
_Not applicable for this compilation phase optimization._

### Manual Verification
- Check the firmware compiles by executing native `pio build`. Validate there are no warnings relative to missing symbols.
- Optionally toggle `CORE_DEBUG_LEVEL` defined value to verify heap allocation modifications natively via `.elf` analysis if the user desires.
