# Context Bridge: RDM API Integration

## 📍 Current State & Focus
We are currently in the planning stage to integrate the National Rail Data Marketplace (RDM) API into the ESP32 Departures Board project. We have researched the memory architecture limitations and drafted an `implementation_plan.md` for a new data provider (`rdmDataSource`) to handle JSON responses natively. We just agreed with the user to proceed by differentiating RDM keys via their stubbed type (`raildata` or `rdm`) in the config framework and using `ArduinoJson`'s filter capability to discard JSON branches efficiently on network streams.

## 🎯 Next Immediate Actions
1. Request the plan queue (`/plan-start`) if the user wants to execute.
2. Begin Execution mapping the JSON payload in `rdmDataSource.cpp`.
3. Construct `rdmDataSource.hpp` deriving from `iDataSource` using ESPAsyncTCP/WiFiClientSecure practices already established in the project.
4. Modify `NationalRailBoard::configure()` to check `ApiKey::type` correctly and instantiate the right derived class.

## 🧠 Decisions & Designs
- **Memory Strategy:** Rather than a whole new streaming parsing library, use `ArduinoJson` passing a `JsonDocument::setFilter()` into `deserializeJson` to effectively ignore all XML-like large payloads outside the exact path arrays we need (`std`, `etd`, `platform`, `trainLength`, `loading`, `isCancelled`).
- **Data Provider Factory:** `NationalRailBoard` will lazily bind either `rdmDataSource` or `nationalRailDataSource` directly onto the heap, caching the pointer (`iDataSource* activeDataSource;`) across the runtime to avoid fragmentation, strictly un-registering the object from `DataManager` on deep reconfiguration. 
- **Configuration Parsing:** We will use the explicit key type, like `raildata` (which is stubbed implicitly upstream via JSON registry) to branch rather than attempting to guess key format string sizes. 

## 🐛 Active Quirks, Bugs & Discoveries
- Single-core ESP32 variants require frequent yielding (e.g., `vTaskDelay(1)`) while parsing huge network chunks. While `deserializeJson` blocks on stream reads, the underlying WiFi stack should ideally be serviced. Ensure `WiFiClientSecure` timeout loops still allow context switches via `esp_task_wdt_reset()` and `vTaskDelay` if parsing takes a while.
- Keep to max ArduinoJson block size of 34KB, meaning filters MUST be strict.
- Double-buffering is used in `iDataSource` implementations (e.g. `NationalRailStation stationData;` vs `renderData;`). The new `rdmDataSource` must retain the background lock logic correctly during parsing to prevent UI tearing.

## 💻 Commands Reference
- IDE command base: PlatformIO `pio run -e esp32dev`
- Testing board: use the portal `/api/keys/test` API if needed. 

## 🌿 Execution Environment
Branch: default or active checked-out feature branch.
Platform: ESP32 using the Arduino Core framework.
