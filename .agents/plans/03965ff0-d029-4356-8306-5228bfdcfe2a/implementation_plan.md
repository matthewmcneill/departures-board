# Yield Callbacks Removal and Multitasking Documentation

This plan removes obsolete `yieldCallback` mechanisms left over from earlier single-threaded code and updates the system documentation to reflect our new background FreeRTOS Data Worker architecture.

## User Review Required

> [!NOTE]
> This removes legacy callbacks that are no longer executed because HTTP and JSON parsing are already securely constrained to `Core 0` on the `Data_Manager` queue. This simplifies function signatures across multiple modules.

## Proposed Changes

### System Definition Documents
*Update reference documents to natively explain the difference between manual yield callbacks and FreeRTOS task handling.*

#### [MODIFY] [AsyncDataRetrieval.md](file:///Users/mattmcneill/Personal/Projects/departures-board/docs/reference/AsyncDataRetrieval.md)
Update the documentation to clarify that `vTaskDelay(1)` is purely an OS-level sleep (yielding Core 0 time back to the scheduler for WiFi/LwIP) and not an application-layer display yield callback.

#### [MODIFY] [DisplaySystemArchitecture.md](file:///Users/mattmcneill/Personal/Projects/departures-board/docs/reference/DisplaySystemArchitecture.md)
Rewrite Section 5 ("Non-Blocking Display Updates") to declare manual yield callbacks as legacy, and explain the new asynchronous WebServer and Multi-Core FreeRTOS execution strategy logic.

#### [MODIFY] [SystemSpecificationDocument.md](file:///Users/mattmcneill/Personal/Projects/departures-board/docs/SystemSpecificationDocument.md)
Remove references to the manual yield pattern from the "System Overview" list, especially concerning `appContext` and `renderAnimationUpdate`.

#### [MODIFY] [WeatherSystemDesign.md](file:///Users/mattmcneill/Personal/Projects/departures-board/docs/reference/WeatherSystemDesign.md)
Update Section 4 to reflect FreeRTOS time-slicing via `vTaskDelay(1)` rather than manual invocations of `yieldCallback`.

---

### Core System Context
*Remove dead pointer functions and delegates meant for the old blocking data fetchers.*

#### [MODIFY] [appContext.hpp](file:///Users/mattmcneill/Personal/Projects/departures-board/modules/appContext/appContext.hpp)
Remove declarations for `yieldCallbackWrapper` and `raildataYieldWrapper`, and remove their associated documentation blocks.

#### [MODIFY] [appContext.cpp](file:///Users/mattmcneill/Personal/Projects/departures-board/modules/appContext/appContext.cpp)
Delete the implementations of `yieldCallbackWrapper` and `raildataYieldWrapper`. Remove the assignments where they are statically injected into the weather and RSS systems.

---

### External Data Clients
*Clean up interfaces by stripping the void pointer callback storage from the data sources.*

#### [MODIFY] [weatherClient.hpp](file:///Users/mattmcneill/Personal/Projects/departures-board/modules/weatherClient/weatherClient.hpp)
Delete the `void (*yieldCallback)()` property and `setYieldCallback` method.

#### [MODIFY] [rssClient.hpp](file:///Users/mattmcneill/Personal/Projects/departures-board/lib/rssClient/rssClient.hpp)
Delete the `void (*yieldCallback)()` property and `setYieldCallback` method.

#### [MODIFY] [tflDataSource.hpp](file:///Users/mattmcneill/Personal/Projects/departures-board/modules/displayManager/boards/tflBoard/tflDataSource.hpp)
Remove `typedef void (*tflDataSourceCallback) ();`, strip the `callback` property natively from the class, and drop the `cb` argument from `configure()`.

#### [MODIFY] [tflDataSource.cpp](file:///Users/mattmcneill/Personal/Projects/departures-board/modules/displayManager/boards/tflBoard/tflDataSource.cpp)
Update the constructor to remove callback initialization, and update `configure()` to match the signature in the header.

#### [MODIFY] [tflBoard.cpp](file:///Users/mattmcneill/Personal/Projects/departures-board/modules/displayManager/boards/tflBoard/tflBoard.cpp)
Remove the `yieldCallbackWrapper` param from both invocations of `dataSource.configure`.

---

### National Rail Sources
*Strip out progress callbacks that belong to the defunct manual yielding logic.*

#### [MODIFY] [nationalRailDataSource.hpp](file:///Users/mattmcneill/Personal/Projects/departures-board/modules/displayManager/boards/nationalRailBoard/nationalRailDataSource.hpp)
Remove the `nrDataSourceCallback` typedef, the pointer variable, and update `init()` to drop the `cb` argument.

#### [MODIFY] [nationalRailBoard.cpp](file:///Users/mattmcneill/Personal/Projects/departures-board/modules/displayManager/boards/nationalRailBoard/nationalRailBoard.cpp)
Remove the `raildataYieldWrapper` argument passed recursively into `dataSource.init`.

## Verification Plan

### Automated Tests
1. **Compilation Validation:**
   Execute standard compilation without hardware target verification.
   Run: `pio run`

### Manual Verification
1. View serial monitor post-flash array validation. The Data Worker should boot and fetch TfL, RSS, and National Rail gracefully without `yieldCallback` interrupts causing any blocking on `Core 1`.
