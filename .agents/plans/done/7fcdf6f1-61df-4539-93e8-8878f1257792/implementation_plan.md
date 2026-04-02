# Migration of Preprocessor Macros to Type-Safe Constants

## Audit Checklist
- [x] Reviewed by `house-style-documentation` - passed
- [x] Reviewed by `architectural-refactoring` - passed
- [x] Reviewed by `embedded-systems` - passed (Resource Impact Assessment added)

> [!NOTE]
> **IP Review Findings**: No warnings generated. Migrating from `#define` to `constexpr` and `enum class` strengthens architectural safety (type scoping) while acting as a provable zero-cost abstraction for the ESP32's RAM/Flash limits.

This plan resolves the "Legacy C-Style Preprocessor Macros (#define)" technical debt identified in the `departures-board` repository. It deprecates the bypassed C++ type system and global scope pollution inherent to `#define`, migrating constants to statically scoped `constexpr` and state codes to rigorously typed `enum class`.

## Proposed Changes

---

### Data Manager Core Interfaces
#### [MODIFY] [iDataSource.hpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/dataManager/iDataSource.hpp)
- Remove `#define TIER_*` and `#define UPD_*` macro definitions.
- Define `enum class PriorityTier : uint8_t { CRITICAL = 0, HIGH = 1, MEDIUM = 2, LOW = 3 };` with Doxygen descriptions.
- Define `enum class UpdateStatus : uint8_t { SUCCESS = 0, NO_CHANGE, NO_DATA, TIMEOUT, HTTP_ERROR, DATA_ERROR, UNAUTHORISED, NO_RESPONSE, INCOMPLETE, PENDING };` with Doxygen descriptions.
- Update core pure virtual method signatures:
  - `virtual UpdateStatus updateData() = 0;`
  - `virtual UpdateStatus testConnection(...) = 0;`
  - `virtual PriorityTier getPriorityTier() = 0;`

---

### Transport Data Sources
#### [MODIFY] [nationalRailDataSource.hpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/boards/nationalRailBoard/nationalRailDataSource.hpp)
#### [MODIFY] [nationalRailDataSource.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/boards/nationalRailBoard/nationalRailDataSource.cpp)
- Remove `NR_MAX_*` macros and replace with `constexpr size_t NR_MAX_LOCATION = 45;`, `constexpr size_t NR_MAX_SERVICES = 9;`, etc.
- Remove `NR_SERVICE_*` macros and replace with `enum class NrServiceType : uint8_t { OTHER = 0, TRAIN = 1, BUS = 2 };`. Create housing Doxygen comment block.
- Update `NationalRailService` struct to use `NrServiceType serviceType`.
- Update overridden method signatures to match `iDataSource.hpp` (`UpdateStatus` / `PriorityTier`).
- Update `volatile int taskStatus` to `volatile UpdateStatus taskStatus`.

#### [MODIFY] [busDataSource.hpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/boards/busBoard/busDataSource.hpp)
#### [MODIFY] [busDataSource.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/boards/busBoard/busDataSource.cpp)
- Convert `BUS_MAX_*` macros to `constexpr size_t`.
- Update overridden method signatures (`updateData`, `testConnection`, `getPriorityTier`) to use the new typed enums.
- Update `volatile int taskStatus` to typed enum.

#### [MODIFY] [tflDataSource.hpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/boards/tflBoard/tflDataSource.hpp)
#### [MODIFY] [tflDataSource.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/boards/tflBoard/tflDataSource.cpp)
- Convert `TFL_MAX_*` macros to `constexpr size_t`.
- Update overridden method signatures (`updateData`, `testConnection`, `getPriorityTier`) to use the new typed enums.
- Update `volatile int taskStatus` to typed enum.

#### [MODIFY] [weatherClient.hpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/weatherClient/weatherClient.hpp)
- Make `getPriorityTier` return `PriorityTier`.

---

### System Controller State Monitors
#### [MODIFY] [systemManager.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/systemManager/systemManager.cpp)
- Update variables storing polling updates (e.g. `lastUpdateResult`) from `int` to `UpdateStatus`.
- Refactor equality checks checking against `9`, `0`, or `UPD_SUCCESS` to instead rigorously check against `UpdateStatus::PENDING`, `UpdateStatus::SUCCESS`, etc.

#### [MODIFY] [displayManager.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/displayManager.cpp)
- Refactor the switch statement `getLastUpdateStatus()` to match strongly typed `UpdateStatus::*` enum variations.

#### [MODIFY] [webHandlerManager.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/webServer/webHandlerManager.cpp)
- Ensure all connection test validation logic (`ds.testConnection()`) parses the `UpdateStatus::SUCCESS` instead of older macros or arbitrary values.

---

### UI Configuration Constants
#### [MODIFY] [iNationalRailLayout.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/boards/nationalRailBoard/iNationalRailLayout.cpp)
- Replace `#define DIMMED_BRIGHTNESS 15` with an anonymous or statically scoped `constexpr uint8_t DIMMED_BRIGHTNESS = 15;`.

#### [MODIFY] [sleepingBoard.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/boards/systemBoard/sleepingBoard.cpp)
- Replace `#define DIMMED_BRIGHTNESS 15` with an anonymous or statically scoped `constexpr uint8_t DIMMED_BRIGHTNESS = 15;`.

#### [MODIFY] [departuresBoard.hpp](file:///Users/mcneillm/Documents/Projects/departures-board/src/departuresBoard.hpp)
- Convert macro configurations `MAX_BOARDS`, `MAX_SCHEDULE_RULES`, and globally used dimensions like `SCREEN_WIDTH` to modern `constexpr` type architectures.

## Resource Impact Assessment
- **Memory (Flash/RAM/Stack/Heap)**: Net zero impact. Both `constexpr` and `enum class` are evaluated purely at compile-time and incur no runtime or allocating cost compared to raw macros.
- **Power**: No impact on battery life, duty cycles, or execution speed.
- **Security**: Upgrading bounds from arbitrary `#define` limits to strictly typed `constexpr size_t` acts as a compile-time guard against array out-of-bounds exploitation.

## Verification Plan

### Automated Tests
- Use PlatformIO to dry-run compile the codebase (`pio run -e [target-environment]`) after completing type replacements to guarantee compile-time enforcement of signatures and that `enum class` usages have fully propagated without causing type decay build failures.

### Manual Verification
- Deploy firmware application to testing board.
- Wait for a boot sequence and verify background workers log expected `UpdateStatus::PENDING` transitioning to `UpdateStatus::SUCCESS` behaviors.
- Log into the Web Configurator and execute a data source validation test (`testConnection`), verifying that errors appropriately parse the updated enums to end clients and return status code 0.
