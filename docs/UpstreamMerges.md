# Upstream Reconciliations & Merges

This document tracks changes published by the original `gadec-uk/departures-board` upstream repository and documents how those modifications have been integrated, adapted, or rejected in this v3.0 object-oriented architecture.

## Version Baseline
- **Architecture Fork Baseline:** `v2.4.6`
- **Latest Upstream Merge Processed:** `v2.4.7`

## v2.4.7 Upstream (Refactored web system to be fully async)

### 1. Asynchronous Web Server Migration
- **Upstream Change:** Replaced standard ESP32 `WebServer` with `ESPAsyncWebServer` and `AsyncTCP` to prevent HTTP requests from blocking the main loop execution.
- **Integration Status:** **ADOPTED & ADAPTED**
- **Details:** The core migration was integrated into `webServer.cpp` and `webHandlerManager.cpp`. By moving to the asynchronous FreeRTOS LwIP event model, the legacy `server.handleClient()` blocking logic was removed from the global `appContext::tick()`. We deeply adapted the payload handling for complex `POST` requests, authoring a dynamic chunking proxy (`bindPostDynamic`) to safely aggregate fragmented firmware payloads and diagnostic UI tools (e.g., Test Keys, Test Boards) before routing the JSON to the application logic state.

### 2. Frontend Upgrade to Bootstrap 5
- **Upstream Change:** Upgraded the legacy captive portal web UI structure to Bootstrap 5.
- **Integration Status:** **REJECTED**
- **Details:** The refactored v3.0 architecture utilizes a completely bespoke, ultra-lightweight Vanilla CSS and JavaScript front-end framework dynamically compiled directly into the binary via `portalAssets.h`. Because our custom UI offers significantly superior hardware performance, thematic alignment, and a vastly smaller memory footprint suitable for ESP32 flash constraints, the upstream Bootstrap 5 refactor was intentionally excluded. Furthermore, embedding the native assets natively guarantees the portal remains fully functional strictly inside Access Point (AP) mode without requiring external CDN requests over the internet.

### 3. National Rail Connection Integrity
- **Upstream Change:** Increased `WiFiClientSecure` thresholds (from 3s to 10s) and internal reading loop iterations to combat arbitrary MbedTLS `-76 (UNKNOWN ERROR CODE)` socket teardown failures.
- **Integration Status:** **ADOPTED**
- **Details:** The timeout adjustments and read buffer logic improvements were fully merged into the `executeFetch()` pipeline inside `nationalRailDataSource.cpp`. This significantly improves firmware stability when the external upstream Darwin endpoint is heavily congested, preventing the ESP32 network driver from arbitrarily discarding the SSL socket mid-transmission.

### 4. Background Data Fetching
- **Upstream Change:** Modified the application behavior to fetch API data recursively for displays that are not currently active in the foreground viewport.
- **Integration Status:** **ADAPTED EXCLUSIVELY**
- **Details:** While the upstream intention was sound, naive background fetching would aggressively exhaust developer API limits under v3.0's expanded multi-board architecture. Instead of merging the upstream logic identically, we engineered a superior **Round-Robin Distributed Matrix** inside `systemManager::tick()`. This algorithm algorithmically staggers background polling intervals (`apiRefreshRate / boardCount`) sequentially across all inactive arrays to protect external server quotas from spiking concurrently. It also introduced a "Fast-Fill" behavior, forcing ultra-low latency updates upon boot synchronization.

### 5. Geospatial Station Picker
- **Upstream Change:** Assessed and patched the NRE Station Picker proxy which broke during the asynchronous networking migration.
- **Integration Status:** **ADOPTED**
- **Details:** We manually restored the legacy endpoint into the async router by overlaying `_server.on("/stationpicker")`. This securely proxies synchronous HTTP clients natively within the `webHandlerManager` namespace to scrape absolute coordinate tracking values (`lat`/`lon`) securely from `ojp.nationalrail.co.uk`.

---
*Note: Maintain this chronological tracking structure for future upstream releases.*

## B2.4-W3.1 Upstream (Commit 4e55b1f)

### 1. Asynchronous Data Transfers on Core 0
- **Upstream Change:** Shifted all network data transfers to run asynchronously on CPU Core 0.
- **Integration Status:** **SUPERSEDED / ALREADY ADAPTED**
- **Details:** The v3.0 architecture has already implemented a far more robust, schedule-driven, priority-aware `DataManager` that serializes network requests on Core 0. The upstream approach is superseded by our custom architecture.

### 2. Bootstrap 5 Alert/Confirm Dialogs
- **Upstream Change:** Switched web admin UI to use Bootstrap 5 modals (`showBsModal`, `bsAlert`, `bsConfirm`) instead of standard browser `alert()` and `confirm()` dialogs.
- **Integration Status:** **REJECTED**
- **Details:** Consistent with the v2.4.7 merge, v3.0 uses a bespoke, ultra-lightweight Vanilla CSS/JS front-end framework engineered for maximum performance and minimal memory footprint on the ESP32. We intentionally exclude Bootstrap 5 and will implement custom vanilla modal overlays if non-blocking dialogs are strictly required by the user flow.

### 3. Tube Mode: Line and Direction Filtering
- **Upstream Change:** Added options to filter London Underground arrivals by specific Tube Line and Direction (Inbound/Outbound/Any). Modifies `TfLdataClient` and web UI.
- **Integration Status:** **REQUIRES ADAPTATION**
- **Details:** A valuable functional addition. This will need to be manually implemented into our `TfLDataSource` component and the corresponding filter settings exposed safely via our custom native web UI.

### 4. Display Layout Updates (Rail Mode) & Service Ordinals
- **Upstream Change:** Tweaked hardcoded display coordinates for Rail mode and added an option to display service ordinal numbers (e.g., "2nd", "3rd", "4th").
- **Integration Status:** **REJECTED (Layouts) / REQUIRES ADAPTATION (Ordinals)**
- **Details:** We reject all hardcoded display layout changes, as v3.0 utilizes a fully dynamic, WASM-capable JSON widget layout registry. The ordinal feature can be adapted by exposing a new configuration boolean and integrating it into our custom `scrollingTextWidget` or `labelWidget` rendering pipelines.

### 5. Service "Last Seen" Location Integration
- **Upstream Change:** Added a configuration option to append the last reported track location (and timestamp) to the scrolling "Calling at" list for National Rail services.
- **Integration Status:** **REQUIRES ADAPTATION**
- **Details:** This enhances the situational awareness of the board. We will need to map the new SOAP properties in our Darwin XML parser and bind them dynamically into our display manager's scrolling text buffer.

### 6. Scrolling Message Pacing Control
- **Upstream Change:** Added a toggle to force the board to wait for a scrolling message (calling points, RSS, service alerts) to fully complete its animation cycle before transitioning to the next primary service.
- **Integration Status:** **REQUIRES ADAPTATION**
- **Details:** Will require integrating a new configuration flag into our display state machine to gate the service transition logic `onCycleComplete`.

### 7. Enhanced Sleep Mode (Complete Display Off)
- **Upstream Change:** Added an option to completely power down the OLED during sleep times instead of showing a screensaver/clock.
- **Integration Status:** **REQUIRES ADAPTATION**
- **Details:** Valuable for maximizing OLED lifespan. Needs to be wired into our hardware-specific driver wrapper (`drawingPrimitives` screen power functions).

### 8. RSS Feed Prioritization
- **Upstream Change:** Option to force RSS headlines to display *before* standard network service messages.
- **Integration Status:** **REQUIRES ADAPTATION**
- **Details:** Will need to be accommodated in our `DataManager` payload structuring and the display manager's message queue logic.

### 9. Weather for Tube Stations
- **Upstream Change:** Enabled the weather widget to function when the board is operating in Tube mode.
- **Integration Status:** **REQUIRES ADAPTATION**
- **Details:** We need to verify that `weatherStatus` fetches correctly using the geospatial coordinates of the selected TfL station.

### 10. Bug Fixes (NTP Startup, Brightness Persistence, OTA Release Notes)
- **Upstream Change:** Improved NTP synchronization sequencing, fixed brightness settings not saving, and added OTA release notes visibility.
- **Integration Status:** **REQUIRES ADAPTATION**
- **Details:** We will audit our `timeManager` and `configManager` for the brightness bug, and adapt the OTA enhancements if they fit within our custom portal workflow.
