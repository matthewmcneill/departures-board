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
