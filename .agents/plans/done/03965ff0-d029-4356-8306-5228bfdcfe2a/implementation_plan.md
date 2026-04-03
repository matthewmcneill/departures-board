[x] Reviewed by house-style-documentation - passed with minor corrections to file paths
[x] Reviewed by architectural-refactoring - passed; removal of legacy callbacks correctly decouples display from data
[x] Reviewed by embedded-systems - passed; zero resource impact, improves code maintainability

# Remove Yield Callbacks & Update Multitasking Docs

This plan removes the legacy `yieldCallback` mechanism from the firmware. These callbacks were previously used to manually trigger display refreshes during blocking network I/O. Since the migration to a dual-core FreeRTOS architecture (Core 0 for data fetching, Core 1 for rendering), these callbacks are redundant and represent technical debt. We will also update the system documentation to reflect the current multitasking architecture.

## User Review Required

> [!IMPORTANT]
> This change simplifies internal API signatures for several data sources. While it reduces boilerplate, it is a breaking change for any out-of-tree modules that might rely on these callback pointers (though none are currently known).

## Proposed Changes

### Core Architecture & AppContext
#### [MODIFY] [appContext.hpp](modules/appContext/appContext.hpp)
- Remove `yieldCallbackWrapper()` and `raildataYieldWrapper()` function declarations.
- Remove `extern appContext* _instance;` since it is exclusively used for yield callbacks.

#### [MODIFY] [appContext.cpp](modules/appContext/appContext.cpp)
- Delete `yieldCallbackWrapper()` and `raildataYieldWrapper()` implementations.
- Remove `_instance = this;` and the static declaration.
- Remove calls to `weather.setYieldCallback()` and `rss.setYieldCallback()`.

---

### Data Sources
#### [MODIFY] [weatherClient.hpp](modules/weatherClient/weatherClient.hpp) / [weatherClient.cpp](modules/weatherClient/weatherClient.cpp)
- Remove `yieldCallback` member and `setYieldCallback` method.
- Remove all invocations of `yieldCallback()` within the JSON parsing loop and network fetch.

#### [MODIFY] [rssClient.hpp](lib/rssClient/rssClient.hpp) / [rssClient.cpp](lib/rssClient/rssClient.cpp)
- Remove `yieldCallback` member and `setYieldCallback` method.
- Remove all invocations of `yieldCallback()` within the XML parsing loop.

#### [MODIFY] [tflDataSource.hpp](modules/displayManager/boards/tflBoard/tflDataSource.hpp) / [tflDataSource.cpp](modules/displayManager/boards/tflBoard/tflDataSource.cpp)
- Remove `tflDataSourceCallback` typedef and `callback` member.
- Update `configure()` signature and implementation to remove the callback parameter.
- Remove all invocations of `callback()`.

#### [MODIFY] [nationalRailDataSource.hpp](modules/displayManager/boards/nationalRailBoard/nationalRailDataSource.hpp) / [nationalRailDataSource.cpp](modules/displayManager/boards/nationalRailBoard/nationalRailDataSource.cpp)
- Remove `nrDataSourceCallback` typedef and `callback` member.
- Update `init()` signature and `setYieldCallback()` (private) to remove callback handling.
- Remove all invocations of `callback()`.

---

### Display Boards
#### [MODIFY] [tflBoard.cpp](modules/displayManager/boards/tflBoard/tflBoard.cpp)
- Update calls to `dataSource.configure()` to remove the `yieldCallbackWrapper` argument.

#### [MODIFY] [nationalRailBoard.cpp](modules/displayManager/boards/nationalRailBoard/nationalRailBoard.cpp)
- Update calls to `dataSource.init()` to remove the `raildataYieldWrapper` argument.

---

### Documentation
#### [MODIFY] [WeatherSystemDesign.md](docs/reference/WeatherSystemDesign.md)
- Delete Section 4 "Non-Blocking I/O (Yield Mechanism)".
- Add a note explaining that weather fetching now runs on Core 0 via `Data_Manager`, leaving the display thread (Core 1) unblocked.

#### [MODIFY] [AsyncDataRetrieval.md](docs/reference/AsyncDataRetrieval.md)
- Update "The Fetch/Yield Cycle" to explain the modern FreeRTOS dual-core relationship and why synchronous yielding is no longer required for display stability.

## Resource Impact Assessment

- **Flash RAM/Heap**: Negligible reduction. Removing static wrappers and member function pointers provides zero-overhead code simplification.
- **Power Consumption**: No impact.
- **Security**: Reduces attack surface by eliminating unnecessary global pointers and callback chains.

## Verification Plan

### Automated Tests
- `pio run`: Ensure the firmware builds correctly with the simplified signatures.
- `pio check`: Perform static analysis to ensure no dead references remain.

### Manual Verification
- `/flash-test`: Flash the device and monitor the serial output.
- Observe that the display animations (scrollers, clocks) remain smooth even during background network activity (e.g., when weather or RSS is updating).
