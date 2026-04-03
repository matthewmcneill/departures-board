# Transitioning to Rail Data Marketplace (RDM) API

## Goal Description
Implement a new `iDataSource` compatible class (`rdmDataSource`) to fetch and parse National Rail departures via the modern Rail Data Marketplace (RDM) REST/JSON API. Modify the `NationalRailBoard` and `ConfigManager` to conditionally utilize this new provider instead of the legacy SOAP/XML `nationalRailDataSource` based on the API key chosen by the user.

> [!IMPORTANT]
> **User Review Required**
> 1. To dynamically select the data provider in `NationalRailBoard`, we will look up the `ApiKey` by its ID and check its `type` attribute (e.g., comparing it against the newly stubbed Rail Data key type, such as `raildata` or `rdm`). The dropdown in the web UI will also filter keys appropriately.
> 2. For memory-efficient JSON parsing, we will use the `ArduinoJson` library along with its native `StaticJsonDocument` or `DynamicJsonDocument` utilizing `deserializeJson` directly from the `WiFiClientSecure` stream. By utilizing ArduinoJson's `JsonDocument::setFilter()` method, parser will discard unneeded JSON branches to preserve the 34KB max heap block size, acting as a highly efficient pseudo-streaming parser without needing an entirely new library.
> 3. The data source instances will be allocated on the heap inside `NationalRailBoard::configure()`. For safe session persistence without fragmentation, an `iDataSource* activeDataSource;` pointer will hold the chosen implementation permanently for the duration of the configuration session, and will only be destroyed and recreated if the board configuration is edited.

## Proposed Changes

### Configuration
#### [MODIFY] `modules/configManager/configManager.hpp`
- Increase `MAX_KEYS` buffer sizes if necessary to accommodate RDM keys, though `char token[64]` is likely sufficient.

### Data Model
#### [NEW] `modules/displayManager/boards/nationalRailBoard/rdmDataSource.hpp`
- Create `rdmDataSource` class inheriting from `iDataSource`.
- Add internal states matching `nationalRailDataSource` buffers (`NationalRailStation`).
- Expose methods to return parsed Data to the board.

#### [NEW] `modules/displayManager/boards/nationalRailBoard/rdmDataSource.cpp`
- Implement `executeFetch()` using `WiFiClientSecure` to `api.raildata.org.uk`.
- Use `x-apikey` HTTP header.
- Use `ArduinoJson` to deserialize JSON payload directly from the network stream. Use a filter document and `deserializeJson(doc, httpClient, filter)` to preserve the heap for large API payloads.
- Map the RDM JSON properties (e.g., `std`, `etd`, `platform`, `trainLength`, `loading`, `isCancelled`) to the common `NationalRailStation` internal tracking structs.

### Display Board Logic
#### [MODIFY] `modules/displayManager/boards/nationalRailBoard/nationalRailBoard.hpp`
- Replace direct member `nationalRailDataSource dataSource;` with an `iDataSource* activeDataSource = nullptr;` pointer.
- Update `updateData()`, `getLastErrorMsg()`, and other getters to delegate tasks to `activeDataSource`.

#### [MODIFY] `modules/displayManager/boards/nationalRailBoard/nationalRailBoard.cpp`
- Modify `configure()` to check the selected `ApiKey`'s `type` field (whether it represents `rail` for legacy XML or the new `raildata` RDM API).
- Initialize and allocate the correct `iDataSource` class (`nationalRailDataSource` or `rdmDataSource`) on the heap to `activeDataSource`. Clean up the old instance if a reconfiguration happens.
- Register and unregister the `activeDataSource` properly with the `DataManager`.

## Verification Plan

### Automated Compilation Tests
- Run `pio run -e esp32dev` to ensure everything compiles and memory footprints are within acceptable limits.

### Manual Verification
- Ask the user to upload firmware to their locally hardware.
- Ask the user to configure a board with an RDM API key. 
- Confirm the board displays trains and checks for RDM specific details such as the presence of train lengths that the legacy payload doesn't populate easily.
