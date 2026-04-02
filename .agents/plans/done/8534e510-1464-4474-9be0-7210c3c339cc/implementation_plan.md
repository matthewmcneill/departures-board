# Decoupling WiFi & WSDL Initialization from UI

You are absolutely right. The UI component (`NationalRailBoard`) shouldn't be responsible for checking WiFi status or triggering blocking SOAP/WSDL initialization logic. This is an architectural smell where the UI layer is bleeding into the network/data layer.

The reason it was implemented this way was because `dataSource.init()` downloads the WSDL API endpoints over HTTPS on first boot. If the UI triggered this synchronously during `onActivate()` before WiFi was connected, it would freeze the renderer. So, a `WiFi.status() == WL_CONNECTED` hack was added to the UI to "defer" the initialization until it was safe.

## Proposed Changes

We can completely eliminate the `WiFi.h` dependency in `NationalRailBoard` and correctly delegate the initialization to the background `DataManager` task (Core 0), which is already designed to handle blocking network operations safely.

***

### modules/displayManager/boards/nationalRailBoard/nationalRailBoard.cpp
#### [MODIFY] nationalRailBoard.cpp

- **Remove:** `#include <WiFi.h>` 
- **Remove:** The entire `WiFi.status() == WL_CONNECTED` checks in `onActivate()` and `updateData()`.
- **Remove:** Calls to `dataSource.init(...)` inside the UI file. The UI will now purely `configure(...)` the tokens and let the Data Source manage itself.
- **Refactor:** `updateData()` to simply ask the `dataSource` to fetch, without trying to manage the WSDL connection state.

***

### modules/displayManager/boards/nationalRailBoard/nationalRailDataSource.cpp
#### [MODIFY] nationalRailDataSource.cpp

- **Refactor:** `nationalRailDataSource::executeFetch()`. This is the method that the `DataManager` executes safely on the background FreeRTOS task.
- **Add:** Logic at the beginning of `executeFetch()` to check if the SOAP endpoint has been cached/discovered yet (i.e., `isInitialized()`). If it hasn't, the background task will call `init(...)` and fetch the WSDL endpoints safely without blocking the UI.

***

### modules/displayManager/boards/nationalRailBoard/nationalRailDataSource.hpp
#### [MODIFY] nationalRailDataSource.hpp

- **Add:** `bool isInitialized() const { return soapAPI[0] != '\0'; }` to allow the data source to determine if it has discovered its endpoints yet.

## User Review Required

This is a clean, surgical change that fixes the exact architectural flaw you pointed out. By shifting the WSDL download to `executeFetch()`, it runs on the background `DataManager` core, completely protecting the UI from freezing. 

Does this plan look good to proceed?
