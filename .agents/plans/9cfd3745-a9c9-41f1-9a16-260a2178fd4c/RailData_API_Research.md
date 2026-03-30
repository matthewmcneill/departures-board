# RailData Assessment vs DARWIN Implementation

This document provides a preliminary assessment of transitioning the `departures-board` firmware from the existing DARWIN SOAP/XML service to the new RailData APIs (such as those introduced via the Rail Data Marketplace).

## 1. Current Architecture (DARWIN SOAP Baseline)
The existing implementation (`nationalRailDataSource.cpp` / `.hpp`) has been heavily optimized for the memory-constrained environment of the ESP32.

### Structural Strengths of Current System
- **Memory Efficiency (Streaming):** It uses `xmlStreamingParser` to process the SOAP response character-by-character as it arrives from the network. It does not buffer the full payload into memory, meaning the heap impact is strictly limited to the `NationalRailStation` struct size (~6.8KB).
- **Concurrency & Hard Real-Time:** Single-core ESP32 variants require frequent yielding. The current parser uses an explicit `yieldCounter` modulo 500 block with `vTaskDelay(1)` to prevent breaking the Wi-Fi stack task watchdogs during parsing.
- **Double Buffering:** The `NationalRailStation` is kept in duplicate (background parser buffer, foreground render buffer) protected by a Mutex (`dataMutex`).

### Current Data Mapping (`NationalRailService` struct)
The following key items are actively parsed from the XML and must find an exact equivalence in the new API:
- **`sTime`**: Scheduled Time (`lt4:std`)
- **`etd`**: Estimated Time (`lt4:etd`)
- **`platform`**: Assigned Platform (`lt4:platform`)
- **`destination`**: Train Destination (`lt4:locationName` inside destination node)
- **`calling`**: Concatenated string of all calling points en-route.
- **`opco`**: Operating Company (`lt4:operator`)
- **`lastSeen`**: Live train location (`lt12:lastReportedStationName`)
- *(Plus Global Board items: `nrccMessages`, `location` for station name)*

## 2. Requirements for the new "RailData" API
Assuming the new RailData API is REST/JSON based (which is standard for the newer RDM architecture), here are the critical impact areas:

### A. Parsing Strategy
- If the new API returns JSON, we cannot easily use a streaming XML parser. We must use `ArduinoJson`.
- **Memory Risk:** A 10-service JSON response with full calling points could easily hit 15KB-25KB. Deserializing this directly into a `JsonDocument` in memory might cause **heap fragmentation or allocation failures** on the ESP32 (especially if running on non-PSRAM models). 
- *Mitigation:* We would need to use `ArduinoJson`'s stream-based deserialization (`deserializeJson(doc, httpsClient)`), using a filter to discard unwanted JSON keys so we don't hold them in the DOM.

### B. Payload Parity
Does the new API provide a single-request "Departure Board with Details" equivalent?
In Darwin, `GetDepBoardWithDetails` returns departures + all subsequent calling stations in one go. If the new RailData API requires 1 request for board, and N requests for calling points (a service details endpoint), that would be catastrophic for the ESP32 network stack.

### C. Authentication Mechanism
- Darwin uses a static token embedded in the XML `SOAP Header`.
- RailData APIs often use `x-api-key` headers or OAuth 2.0 Bearer tokens. 
- *Impact:* We need to ensure the firmware configuration manager is updated to accept the new token format and potentially handle token refresh lifecycles if it uses short-lived tokens.

## 3. Investigatory Questions (Answered)
We conducted a live exploration of the Rail Data Marketplace (RDM) developer portal and successfully identified the modern REST API equivalents for DARWIN:

1. **Endpoint & Format:** The active REST API is provided under the "Live Arrival and Departure Boards" product. 
   - **Method/URL:** `GET https://realtime.nationalrail.co.uk/LDBWS/api/20220120/GetDepBoardWithDetails/{crs}`
   - **Format:** JSON
   - **Auth:** Standard `Authentication` HTTP header.
2. **One-Shot vs Multi-Shot (Payload Parity):** The new REST API **maintains full parity** with the old SOAP service. A single call to `GetDepBoardWithDetails` returns the departures array, and crucially, each nested service object natively embeds an array of `subsequentCallingPoints` and `previousCallingPoints`. This means we will not need to make multi-shot service requests!
3. **Data Sparseness ("Last Seen" feature):** There is no single `.lastSeen` string, but the positional tracking data is provided natively within the `previousCallingPoints` array. Each calling point contains `.at` (Actual Time) and `.et` (Estimated Time). We can implement the "Last Seen" feature by iterating the previous calling points and selecting the most recent location with a valid `.at` timestamp.
4. **Rate Limits & Polling:** While the exact quota limits depend on the specific user subscription (Open Licence vs Commercial), the API guarantees 96.83% minimum uptime. We will maintain our 45-second polling interval to respect general rate limits and protect battery/thermal profiles.
5. **Payload Size (Outstanding):** Before we can draft the final `implementation_plan.md` (which requires ESP32 memory resource impact evaluations per project rules), we need to capture a real JSON payload for a major station during service to calculate the exact `ArduinoJson` buffer length.
