# Implementation Plan: Upstream B2.3-W3.0 Porting

## Goal Description
The objective is to carefully port the major improvements from upstream release [B2.3-W3.0] directly into our new v3.0 Object-Oriented modular architecture. 

Reviewing the provided GitHub release tag, there are two major architectural adjustments we must port:
1. **National Rail "no data" bug**: The upstream fix simply increases the `WiFiClientSecure` thresholds. The API occasionally takes >5 seconds to generate the XML, so the update bumps the hard connection timeout to 10s and the read loop limit from 30 iterations (3s) to 80 iterations (8s).
2. **Asynchronous Web Server**: Upstream migrated off the blocking ESP32 `WebServer` standard library onto `ESPAsyncWebServer`. This improves UI responsiveness significantly because HTTP requests no longer stall the main `loop()` animation tick.
# Phase 4: Background Service Synchronization & UI Display Readiness

This change modifies how the system fetches backend data. Instead of keeping inactive boards "asleep" until they rotate into view, we will execute backend polling on a synchronized schedule for *all* configured boards continuously. This completely eradicates visual pop-in latency when transitioning between custom displays. It also intelligently overrides the "No Services Found" widget during boot to display a distinct "Loading data..." status while the first async sweeps run.

## User Review Required

> [!CAUTION]
> **API Quota Implications**: By moving from "Active Board" fetching to "All Boards Background" fetching, the system will fire parallel API requests. If you have 6 customized boards, the ESP32 will routinely fire 6 separate SSL requests against National Rail, TfL, and Bus endpoints within close succession.
> * National Rail (Darwin) is typically flexible, but TfL has rate limits on their free tier.
> Ensure that multiple active queries do not aggressively bottleneck your configured developer quotas!

## Proposed Changes

### Configuration State Manager
Migrating a singular polling clock to a distributed hardware array matrix.

#### [MODIFY] [systemManager.hpp](file:///absolute/path/to/modules/systemManager/systemManager.hpp)
- Add `int backgroundUpdateIndex = 0;` to store the rotational queue index.
- Rename `nextDataUpdate` to `nextRoundRobinUpdate` maintaining a singular timing loop.

#### [MODIFY] [systemManager.cpp](file:///absolute/path/to/modules/systemManager/systemManager.cpp)
- **`tick()` Round-Robin Tracking Loop**:
  - Keep the explicit fast-fetch priority logic identifying when `activeIndex` changes (improving zero-latency visual switches).
  - Rather than firing arrays consecutively, calculate a `distributedInterval` based on `(config.apiRefreshRate / config.boardCount)`.
  - **Fast-Fill Initialization**: If the background target board has never successfully loaded data (`lastUpdateStatus == -1`), bypass the long `distributedInterval` and instead queue the next board after a short `2000ms` window. This ensures all boards are "Fast-Filled" at boot rapidly before settling into the relaxed quota-friendly round-robin rotation.
  - Stagger the logic: Every `distributedInterval` (or `Fast-Fill`) tick, advance `backgroundUpdateIndex = (backgroundUpdateIndex + 1) % config.boardCount;` and `updateData()` on that singular specific hardware slot natively inside the `dataWorker` queue.
  - Apply `distributedInterval` min-max bounds (e.g., hard cap `10,000ms` lowest delay context constraint) to throttle API requests.

---

### Hardware Rendering Implementations
Establishing fallback widget overrides when the API request falls out of bounds or is explicitly caught in the `UPD_PENDING` / Initialization pipeline context.

#### [MODIFY] [nationalRailBoard.cpp](file:///absolute/path/to/modules/displayManager/boards/nationalRailBoard/nationalRailBoard.cpp)
- Mutate `render()` logical fallbacks. If `numServices <= 0`, dynamically re-read `lastUpdateStatus`:
   - If `-1` (Unitialized) or `9` (`UPD_PENDING`), override `noDataLabel` payload strictly to `"Loading data..."`.
   - Else, assume literal 0 count response matching source rules and restore `"No services found."`.

#### [MODIFY] [tflBoard.cpp](file:///absolute/path/to/modules/displayManager/boards/tflBoard/tflBoard.cpp)
- Repeat mutation logic inside `render()`. TfL applies label: `"Loading data..."` mapping natively against `"No arrivals scheduled."`

#### [MODIFY] [busBoard.cpp](file:///absolute/path/to/modules/displayManager/boards/busBoard/busBoard.cpp)
- Repeat mutation logic inside `render()`. Bus Board applies mapping directly referencing `"No scheduled services."`

## Verification Plan

### Automated Tests
- Validate matrix compilation via `pio run -e esp32dev` mapping strict namespace boundaries on `MAX_BOARDS`.

### Manual Verification
1. Reboot the hardware onto the network. The very first drawn screen for all arrays should momentarily indicate exactly: "Loading data...".
2. Carousel explicitly off Board 1 -> Board 3. 
   - Observe visual artifact payload representation over serial verifying if Board 3 immediately repaints services (avoiding latency pop-in).
