# Plan Audit Checklist
- [x] Architectural Standards (`oo-expert`)
- [x] House Style (`house-style-documentation`)
- [x] UI Design (`embedded-web-designer`) - N/A
- [x] Resource Impact (`embedded-systems`)

# Transitioning to Rail Data Marketplace (RDM) API

## Goal Description
Implement a new `iDataSource` compatible class (`nrRDMDataProvider`) to fetch and parse National Rail departures via the modern Rail Data Marketplace (RDM) REST/JSON API. Modify the `NationalRailBoard` and `ConfigManager` to conditionally utilize this new provider instead of the legacy SOAP/XML provider (which will be renamed to `nrDARWINDataProvider`) based on the API key chosen by the user.

> [!IMPORTANT]
> **User Review Required**
> 1. To dynamically select the data provider in `NationalRailBoard`, we will look up the `ApiKey` by its ID and check its `type` attribute (e.g., comparing it against the newly stubbed Rail Data key type, such as `raildata` or `rdm`). The dropdown in the web UI will also filter keys appropriately.
> 2. For memory-efficient JSON parsing, we will use the `ArduinoJson` library along with its native `StaticJsonDocument` or `DynamicJsonDocument` utilizing `deserializeJson` directly from the `WiFiClientSecure` stream. By utilizing ArduinoJson's `JsonDocument::setFilter()` method, parser will discard unneeded JSON branches to preserve the 34KB max heap block size, acting as a highly efficient pseudo-streaming parser without needing an entirely new library.
> 3. The data source instances will be allocated on the heap inside `NationalRailBoard::configure()`. For safe session persistence without fragmentation, an `iDataSource* activeDataSource;` pointer will hold the chosen implementation permanently for the duration of the configuration session, and will only be destroyed and recreated if the board configuration is edited.

## Codebase Architectural Evaluation (@[.agents/skills/oo-expert])

**1. Modular Structure & Abstraction**
- *Issue*: Currently, `nationalRailDataSource` couples DARWIN SOAP parsing logic (`xmlListener`) tightly with the underlying domain models (`NationalRailStation`). `NationalRailBoard` cannot consume `iDataSource` directly because it requires `getStationData()` which isn't part of the base `iDataSource` interface.
- *Recommendation*: Extract the domain structs directly into an intermediate abstract interface `iNationalRailDataProvider` (inheriting from `iDataSource`). Both DARWIN and RDM data sources will implement this contract, satisfying the Interface Segregation and Dependency Inversion Principles, while avoiding extra header files.

**2. Train Formation Data Modeling**
- *Issue*: RDM supplies detailed train formation data (coaches, loading) that DARWIN omitted, but our current structs don't capture this.
- *Recommendation*: Expand `NationalRailService` to safely encapsulate an array of coach formation details, ensuring fixed maximum capacities so we do not cause heap fragmentation on the ESP32.

## Proposed Changes

### Data Interface Layer

#### [NEW] [iNationalRailDataProvider.hpp](modules/displayManager/boards/nationalRailBoard/iNationalRailDataProvider.hpp)
- Abstract interface overriding `iDataSource`.
- Move `NationalRailService` and `NationalRailStation` structs here from the old `nationalRailDataSource.hpp`.
- Add new `struct NrCoachFormation { char coachClass; char loading[16]; uint8_t coachNumber; };`
- Add an array `NrCoachFormation coaches[MAX_COACHES];` and `uint8_t numCoaches;` to `NationalRailService`.
- Defines contracts: `virtual NationalRailStation* getStationData() = 0;`, `virtual MessagePool* getMessagesData() = 0;`, and `virtual void configure(...) = 0;`.

### Data Providers

#### [MODIFY] [nrDARWINDataProvider.hpp](modules/displayManager/boards/nationalRailBoard/nrDARWINDataProvider.hpp) & [nrDARWINDataProvider.cpp](modules/displayManager/boards/nationalRailBoard/nrDARWINDataProvider.cpp)
- Rename from `nationalRailDataSource` to `nrDARWINDataProvider` across files and classes.
- Inherit from `iNationalRailDataProvider` instead of `iDataSource`.
- Include `iNationalRailDataProvider.hpp` and remove the local struct definitions.

#### [NEW] [nrRDMDataProvider.hpp](modules/displayManager/boards/nationalRailBoard/nrRDMDataProvider.hpp) & [nrRDMDataProvider.cpp](modules/displayManager/boards/nationalRailBoard/nrRDMDataProvider.cpp)
- Create `nrRDMDataProvider` class inheriting from `iNationalRailDataProvider`.
- Implement `executeFetch()` using `WiFiClientSecure` to `api.raildata.org.uk` using `x-apikey`.
- Map the RDM JSON properties securely using `ArduinoJson` to the updated `NationalRailStation` structs, including mapping any parsed formation data into the newly added `coaches` array.

### Display Board Logic

#### [MODIFY] [nationalRailBoard.hpp](modules/displayManager/boards/nationalRailBoard/nationalRailBoard.hpp)
- Replace direct member `nationalRailDataSource dataSource;` with an `iNationalRailDataProvider* activeDataSource = nullptr;` pointer.
- Update delegation methods to point to `activeDataSource`.

#### [MODIFY] [nationalRailBoard.cpp](modules/displayManager/boards/nationalRailBoard/nationalRailBoard.cpp)
- Modify `configure()` to instantiate `nrDARWINDataProvider` or `nrRDMDataProvider` onto the heap in `activeDataSource` based on tracing the chosen key type from `config.apiKeyId`.

### Configuration and Web UI Flow

#### [MODIFY] `web/index.html` (Web Portal)
- **Key Registration**: Introduce a designated `'rdm'` Key creation card alongside the existing `'rail'` card.
- **Board Configuration Dropdown**: Modify the Dropdown constraint check (`targetType`) to allow both `'rail'` AND `'rdm'` keys to be selected when configuring a Rail board.
- **Station Coordinates Pathway**: Trace the `lookupCoords()` flow. Since it relies on the TfL National Rail fallback system, explicitly allow `'rdm'` typed keys to use this same fallback since both DARWIN and RDM use identical CRS codes.

#### [MODIFY] [webHandlerManager.cpp](modules/webServer/webHandlerManager.cpp)
- Upgrade `ApiTestDataSource::executeFetch()` within the `/api/keys/test` API endpoint.
- Add an explicit `} else if (params->type == "rdm") {` branch.
- Instantiate an isolated instance of `nrRDMDataProvider` and invoke its `testConnection()` dynamically to validate the RDM API token, keeping the test flow identical to DARWIN.

## Resource Impact Assessment (@[.agents/skills/embedded-systems])

> [!WARNING]
> - **Heap / RAM**: `ArduinoJson` will be restricted to a single pre-allocated `StaticJsonDocument` or heavily filtered `DynamicJsonDocument` (reusing a fixed stream) to prevent fragmentation. Expanding `NationalRailService` to hold formation data increases its base struct size. With multiple services (`MAX_SERVICES`) and double buffering via `std::unique_ptr` used by the data layer, we must ensure the `coaches` array size (e.g., `MAX_COACHES = 12`) per service does not exceed the remaining typical free heap block size (~30-40KB) on standard ESP32s during network workloads.
> - **Flash / ROM**: Incorporating `ArduinoJson` statically directly against `WiFiClientSecure` will slightly increase binary size but will not threaten the OTA partition boundaries.
> - **Execution Time**: Yield checks (`vTaskDelay(1)` or `esp_task_wdt_reset()`) are strictly required through the JSON deserialization loop to prevent Watchdog Timer (WDT) resets on single-core variants.

## Verification Plan

### Automated Compilation Tests
- Run `pio run -e esp32dev` to ensure everything compiles and memory footprints are within acceptable limits.

### Manual Verification
- Ask the user to upload firmware to their locally hardware.
- Ask the user to configure a board with an RDM API key. 
- Confirm the board displays trains and checks for RDM specific details such as the presence of train lengths that the legacy payload doesn't populate easily.
