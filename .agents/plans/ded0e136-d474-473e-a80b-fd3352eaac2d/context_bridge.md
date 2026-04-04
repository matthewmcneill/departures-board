# Context Bridge

## 📍 Current State & Focus
We have completed the core implementation of the Rail Data Marketplace (RDM) transition for the National Rail departures board. A polymorphic data architecture has been implemented (`iNationalRailDataProvider`), dividing the implementation into `nrDARWINDataProvider` (legacy XML) and `nrRDMDataProvider` (new JSON parser using safe streaming and ArduinoJson `Filter`). The `NationalRailBoard` has been decoupled to dynamically instantiate the appropriate provider depending on the selected user configuration. The `index.html` frontend components for adding RDM API keys have been completed. The firmware code was compiling successfully but threw an exit code 1 linked to `TimeLib.h` which was removed as it was unnecessary. Further compilation needs validation. Hardware and UI tests are pending.

## 🎯 Next Immediate Actions
1. Re-run `pio run -e esp32dev` to verify the codebase compiles successfully after removing the invalid `TimeLib.h` include from `nrRDMDataProvider.cpp` and finalizing the DataManager pointer fixes.
2. The user will invoke `/flash-test` using an active RDM API key to verify behavior on physical hardware.
3. Validate memory efficiency using the serial read tools to ensure peak memory doesn't cross the 34KB free heap limit during an RDM JSON parsing cycle.
4. Verify web portal RDM configuration layout visually.

## 🧠 Decisions & Designs
- **Polymorphic Interfaces**: Created `iNationalRailDataProvider` sharing functionality for the board display loop while separating API fetching mechanics into `nrDARWINDataProvider` and `nrRDMDataProvider`. 
- **Memory Bound Parsing**: To handle the very large RDM JSON responses, `ArduinoJson::DeserializationOption::Filter` was used in a streaming context directly off `WiFiClientSecure` rather than buffering `String` objects, avoiding memory exhaustion.
- **Provider Registration**: Fixed lifecycle bugs in `nationalRailBoard.cpp` that previously assumed static `dataSource` addresses, ensuring cleanup handles the dynamic `activeDataSource` pointer gracefully.

## 🐛 Active Quirks, Bugs & Discoveries
- Need to ensure `JsonArray` parsing iterates fully through RDM `trainServices` and extracts coach formation correctly if added later. Current JSON filtering is highly tuned to match the legacy XML data density exactly.
- PlatformIO occasionally errors out if `ESPAsyncWebServer` warnings trip up CI but warnings shouldn't fail compilation; the previous failure was verified as a `No such file` for `TimeLib.h`.

## 💻 Commands Reference
- To build the firmware: `pio run -e esp32dev`
- To flash and monitor logs (requires lock/device): `/flash-test`
- Monitor continuous serial: `/monitor`

## 🌿 Execution Environment
- Building on ESP32 environment via PlatformIO.
- Physical testing pending connection with active RDM key.
