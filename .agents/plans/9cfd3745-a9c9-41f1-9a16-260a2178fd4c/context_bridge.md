# Context Bridge: RailData REST API Integration

## 📍 Current State & Focus
We are at the beginning stages of migrating the departure board firmware from the legacy DARWIN SOAP XML API to the modern Rail Data Marketplace (RDM) REST JSON API. We have completed deep browser research on the developer portal (raildata.org.uk) and mapped the JSON structures to our memory constraints. We are currently blocked on needing a live JSON payload to profile the `ArduinoJson` memory footprint on ESP32.

## 🎯 Next Immediate Actions
1. Secure the physical API Token for the Rail Data Marketplace from the user.
2. Execute a raw HTTP request (e.g. `curl`) to pull live JSON data for a heavy-traffic station like Paddington (`PAD`) or Waterloo (`WAT`).
3. Analyze the JSON response footprint and draft an `implementation_plan.md` that proves `ArduinoJson` will fit in ESP32 RAM under load without fragmentation.
4. Finalize the `nationalRailDataSource` JSON refactor plan.

## 🧠 Decisions & Designs
- **Endpoint:** Moving to `GET /LDBWS/api/20220120/GetDepBoardWithDetails/{crs}`.
- **Payload Parity:** We discovered that a single network call returns the departures AND their `subsequentCallingPoints`, so we do NOT need to implement multi-shot logic. This saves massive overhead.
- **Last Seen Emulation:** We will implement "Last Seen" natively by iterating the `previousCallingPoints` array and picking the final valid `at` (Actual Time) node.

## 🐛 Active Quirks, Bugs & Discoveries
- **Hardware Memory Threat:** Reverting from a streaming XML parser to `ArduinoJson` is a risk on non-PSRAM ESP32 models. We need a tight `ArduinoJson` filter when parsing the stream (`deserializeJson(doc, client)`) so we don't accidentally buffer redundant JSON nodes.

## 💻 Commands Reference
None currently executed. The next step is a live `curl` against the real API.
Example: `curl -H "Authentication: YOUR_TOKEN" "https://realtime.nationalrail.co.uk/LDBWS/api/20220120/GetDepBoardWithDetails/PAD?numRows=10"`

## 🌿 Execution Environment
- No code committed or branched yet.
- Awaiting user input on the API key.
