# Configuration Data Audit & Reference

This document provides a rigorous map of all configurable fields within the departures-board web portal, tracing them from the UI down to the underlying JSON file storage and evaluating how cleanly the consuming C++ modules implement them.

## 1. Summary Configuration Map

This map traces every active configuration field from its exact location in the Web Portal (including nested sub-dialogs) to the physical JSON file and key.

| Web Portal Tab | Sub-Dialog / Element | Parameter Name / UI Label | JSON File | JSON Key | Major Consumer Modules |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **System Settings** | *Main Tab* | Hostname (mDNS) | `/config.json` | `hostname` | `wifiManager`, `appContext` |
| | | Timezone | `/config.json` | `TZ` | `timeManager` |
| | | Global Brightness | `/config.json` | `brightness` | `displayManager`, `hwDriver` |
| | | Show Date on Screen | `/config.json` | `showDate` | `iDisplayBoard` implementations |
| | | Suppress Scrolling & Info | `/config.json` | `noScroll` | `displayManager` |
| | | Increase API Refresh Rate | `/config.json` | `fastRefresh` | `dataManager` |
| | | Flip Screen (180°) | `/config.json` | `flip` | `hwDriver` (OLED Driver) |
| | | Wait for scroll complete | `/config.json` | `waitForScroll` | `displayManager` (Carousel) |
| | | Show RSS headlines first | `/config.json` | `rssFirst` | `displayManager` |
| | | Enable Firmware Updates | `/config.json` | `update` | `appContext` |
| | **WiFi Network Dialog** | SSID | `/wifi.json` | `ssid` | `wifiManager` |
| | | Password | `/wifi.json` | `password` | `wifiManager` |
| **Web & Weather** | *Main Tab* | Global Weather Key | `/config.json` | `weatherKeyId` | `weatherClient` |
| | **Add Feed Dialog** | XML Feed URL | `/config.json` | `rssUrl` | `newsDataSource` |
| **Scheduling Tab**| *Main Tab* | Carousel Interval | `/config.json` | `carouselInterval` | `displayManager` |
| | | Auto-override timeout | `/config.json` | `overrideTimeout` | `displayManager` |
| | **Edit Schedule Dialog**| Start Time | `/config.json` | `schedules[].startH`/`M` | `schedulerManager` |
| | | End Time | `/config.json` | `schedules[].endH`/`M` | `schedulerManager` |
| | | Target Board | `/config.json` | `schedules[].board` | `schedulerManager` |
| **Boards (Displays)** | **Edit Board Dialog** | Transport Mode | `/config.json` | `boards[].type` | `BoardFactory`, `configManager` |
| | | Display Name | `/config.json` | `boards[].name` | UI, `iDisplayBoard` Headers |
| | | Station/Stop ID (CRS/Naptan) | `/config.json` | `boards[].id` | `iDataSource` Implementations |
| | | Display Layout | `/config.json` | `boards[].layout` | `iDisplayBoard` renderer |
| | | Board API Key | `/config.json` | `boards[].apiKeyId` | `dataManager` |
| | | Platform Filter | `/config.json` | `boards[].filter` | Data Sources |
| | | Calling Station (Secondary) | `/config.json` | `boards[].secId` / `secName` | Rail Data Source |
| | | Latitude / Longitude | `/config.json` | `boards[].lat` / `lon` | `weatherClient` |
| | | Board Brightness Override | `/config.json` | `boards[].brightness` | `displayManager` |
| | | Show Weather Overlay | `/config.json` | `boards[].weather` | `displayManager` |
| | | TfL Line | `/config.json` | `boards[].tflLine` | `tflDataSource` |
| | | TfL Direction | `/config.json` | `boards[].tflDir` | `tflDataSource` |
| | | Show Service Ordinals | `/config.json` | `boards[].ordinals` | Rail / Tube Boards |
| | | Last Seen Location | `/config.json` | `boards[].lastSeen` | Rail Board |
| | | Turn OLED Off in Sleep | `/config.json` | `boards[].oledOff` | `schedulerManager`, `hwDriver` |
| **API Keys** | **Add / Edit API Key** | Key Label | `/apikeys.json` | `keys[].label` | UI Only (Visual sorting) |
| | | Token string | `/apikeys.json` | `keys[].token` | API Request Builders |
| | | Key Type | `/apikeys.json` | `keys[].type` | UI Only (categorization) |

---

## 2. In-Depth Board Configuration Analysis (Overlaps & Redundancies)

The `boards[]` array captures a "God Object" payload containing every possible permutation of settings across all board types. However, depending on the Transport Mode (`type`), the C++ modules actively ignore vast swaths of these fields. 

Below is an exhaustive analysis of how the fields map contextually to each specialized board, revealing architectural redundancies and UI confusion points.

### 2.1 Transport for London (Tube) Board (`MODE_TUBE`)
*   **Station/Stop ID** (`id`): TfL Naptan ID (e.g., `940GZZLUTCR`). The C++ backend features a hardcoded workaround to instantly convert hub codes (e.g. `HUBTCR`) into literal Naptan IDs to prevent crashes.
*   **API Key ID** (`apiKeyId`): Linked correctly to the registry. Injected to `tflDataSource`.
*   **TfL Line** (`tflLine`): A direct substring filter on incoming parsed JSON payloads. Flushes out array elements containing strings that don't match.
*   **TfL Direction** (`tflDir`): Integer casting (1 = Inbound, 2 = Outbound). Operates a bespoke substring match against words like "southbound", "westbound", etc in the generic `platformName` field.
*   **Display Name** (`name`): Maps accurately to the location header logic since Naptan IDs do not provide clean headers natively via standard arrays.
*   **Show Service Ordinals** (`ordinals`): Injects "1", "2" order sequence natively into the 4-Column rendering loop.
*   **Show Weather Overlay** (`weather`) & **Coords**: Used for fetching macro-climate overlays for the individual station.
*   **Redundancies & Blank Fields**:
    *   **Filter** (`filter`): Currently wholly ignored. The layout enforces `setFilters("")` universally, so entering Platform strings in this field does nothing.
    *   **Display Layout** (`layout`): The UI permits layout selection, but `tflBoard.cpp` overrides this unequivocally with `activeLayout = new layoutTflDefault(context);`
    *   **Secondary ID / Name**: Native Rail concepts completely ignored by the TfL parser.
    *   **Show Last Seen at...** (`lastSeen`): Ignored.

### 2.2 National Rail Board (`MODE_RAIL`)
*   **Station/Stop ID** (`id`): Evaluated strictly as a 3-letter CRS code.
*   **API Key ID** (`apiKeyId`): Actively used. Maps to Darwin WSDL token.
*   **Filter** (`filter`): Evaluated successfully natively. If present, it enforces platform-level filtering. Natively creates the sub-header (e.g. "[Plat X]").
*   **Secondary ID / Secondary Name** (`secId` / `secName`): Triggers the "Calling at..." routing behavior cleanly utilizing Darwin via points.
*   **Display Layout** (`layout`): Properly maps between `replica` (creates `layoutNrReplica`) and fallback default layouts.
*   **Show 'Last Seen at...'** (`lastSeen`): Evaluated reliably to append Darwin `lastReportedStationName` tracking strings cleanly to the UI message ticker.
*   **Redundancies & Blank Fields**:
    *   **Display Name** (`name`): Entirely ignored. The user sets a name in the portal, but `nationalRailBoard.cpp` forcefully overwrites the display header with the official name (`data->location`) returned from the live Darwin API endpoint.
    *   **TfL Line** (`tflLine`) & **TfL Direction** (`tflDir`): Ignored.

### 2.3 Local Bus Board (`MODE_BUS`)
*   **Station/Stop ID** (`id`): Transport ATCO code.
*   **Display Name** (`name`): Cleanly applied to the location header, unlike National Rail.
*   **Filter** (`filter`): Operates uniquely against buses. Converts a CSV text string (e.g. "86, 12, N86") into a route whitelist. Evaluated directly in `busDataSource::serviceMatchesFilter`.
*   **Redundancies & Blank Fields**:
    *   **Board API Key**: Totally ignored. The `bustimes.org` proxy does not natively authenticate with tokens. The Web Portal UI exposes the dropdown unnecessarily. 
    *   **Display Layout** (`layout`): The UI permits layout selection, but `busBoard.cpp` overrides this unequivocally with `activeLayout = new layoutBusDefault(context);`.
    *   **Secondary ID / Secondary Name**: Ignored.
    *   **Show Last Seen**: Ignored.
    *   **TfL Line** (`tflLine`) & **TfL Direction** (`tflDir`): Ignored.

### 2.4 Standard Clock Board (`MODE_CLOCK`)
*   **Turn OLED Off in Sleep** (`oledOff`): The only specifically checked boolean field evaluated during the global Display Manager provisioning loop: `if (sb) sb->setOledOff(config.boards[i].oledOff);`.
*   **Redundancies & Blank Fields**:
    *   Every single other field (coords, IDs, layouts, keys, filters) saved against a Clock Board struct is dead weight.

---

## 3. General Parameter Implementation Audit

### 3.1 System Settings (`/config.json`)
- **Timezone (`TZ`)**:
  - **Purpose**: Defines the POSIX timezone mapping to ensure scheduling rules run in local time correctly, handling DST transitions.
  - **Evaluation**: Exceptionally implemented. Hooked directly into `timeManager.cpp` and `setenv("TZ", ...)`, pushing timezone management robustly down to the native `time.h` libraries.
- **Refresh Rate (`fastRefresh`)**:
  - **Purpose**: Lowers the data-fetch interval.
  - **Evaluation**: Functional but relies on a boolean checkbox on the UI rather than a formal explicit interval dropdown.
- **Wait for Scroll (`waitForScroll`)**:
  - **Purpose**: Prevents the board carousel from switching eagerly while a long notification is scrolling.
  - **Evaluation**: Very well integrated. Evaluated continuously within the `displayManager` render loop, cleanly delaying state transitions. 

### 3.2 WiFi configuration (`/wifi.json`)
- **SSID & Password**:
  - **Evaluation**: Works reliably, but stores raw plaintext credentials in LittleFS. Because endpoints and APs provide local unencrypted retrieval payloads across the filesystem, this compromises Wi-Fi password integrity. 

### 3.3 API Keys Registry (`/apikeys.json`)
- **Array of { `id`, `label`, `type`, `token` }**:
  - **Evaluation**: The v2.0 "Registry Method" is an excellent architectural choice. It decouples the UI instances from the credentials themselves so that multiple bus stops can be dynamically configured without inputting identical tokens. The migration logic residing in `configManager` handles legacy flat payloads perfectly.

### 3.4 Geographic Data (`lat`, `lon`, `weather`)
- **Evaluation**: Geolocation variables are correctly saved and consumed on a strictly "per board" basis. This handles the architectural reality that a user may configure boards for physically distributed stations spanning large macro-climate zones.

### 3.5 Time Scheduling (`/config.json` -> `schedules[]` array)
- **Evaluation**: Functional but primitive. The UI presents an array, and changes are validated loosely. On the C++ side, `MAX_SCHEDULE_RULES` restricts this tightly. If the user posts "25" rules, the deserializer silently omits them, which might confuse the user. 

---

## 4. Internal Orphan & Legacy Variables Audit (100% Coverage)

As part of securing a 100% comprehensive audit over the data footprint, the following JSON keys are read and written continuously by `configManager.cpp` during C++ state synchronisation, but are inherently **Orphaned** (They possess no mapped interface in `web/index.html` to be updated natively by the user), or are **Legacy Migrations**:

*   **`version`**: The internal C++ migration state (e.g. `2.4`). Handled exclusively by the backend upgrade scripts.
*   **`sleep`**, **`sleepStarts`**, **`sleepEnds`**, **`clock`**: Legacy global sleep variables. Replaced in `v2.4` by the dynamic `schedules[]` UI and `MODE_CLOCK` boards. The frontend actively captures old variables during rendering by executing `migrateLegacySleep()` and converting them into new arrays, but the C++ backend continues padding the struct memory to preserve backwards-serialization support.
*   **`mode`**: Internally maps to `defaultBoardIndex`. This was the legacy index pointer dictating which single board to show on boot before the "Carousel" workflow was instantiated. Kept alive in memory but stripped structurally from the Web UI.
*   **`updateDaily`**: Present in the JSON ingestion struct (`config.dailyUpdateCheckEnabled`) to ping OTA endpoint at midnight, but the HTML page solely features a monolithic `firmwareUpdatesEnabled` (`update`) root toggle switch.
*   **`rssName`**: Retained in the C++ backend structs for feed context, but `index.html` calculates titles purely via front-end select dropdowns.
*   **`boards[].offset` / `timeOffset`**: Included heavily in `nationalRailDataSource.cpp` logic to spoof current time (useful for debugging early morning API returns). It is securely retained by a JavaScript `hidden` state (`editingBoardOffset`) upon `saveConfig()`, but the Edit Board UI sub-dialog purposefully hides it.

---

## 5. Critical Evaluation & Recommended Actions

The configuration and persistence architecture is highly mature. Moving to isolated registries for API keys & WiFi sets the firmware apart from "hobbyist" grade and closer to "product" grade operations. However, memory size limitations and serialization flaws exist and should be reviewed:

### Recommendation 1: Dynamic UI Toggling based on Mode
The Javascript (`app.populateSettings()`) must be refactored to aggressively toggle `<div class="board-config-row">` elements based on the `target.value` of the Transport Mode selector. Exposing "Tfl Line", "Platform Filter", and "API Key" options when a user has clicked "Clock" or "Bus" creates a confusing user experience and leads to massive overlap/abandoned JSON payloads.

### Recommendation 2: Move Secrets to ESP32 Hardware NVS
Right now, reading files from LittleFS returns raw JSON variables natively. 
- **Action**: Deprecate `/wifi.json` entirely and save wifi strings in standard `Preferences.h` / ESP-NVS partition memory. Do the same for Token values. LittleFS should only hold benign view configurations like `brightness` or `board[]` layouts.

### Recommendation 3: Implementation of JSON Schema Guardrails in 'Save' APIs
When `webHandlerManager.cpp` accepts API modifications, `DeserializationError error = deserializeJson(doc, request->arg("plain"));` takes place. Little manual constraint checking exists to ensure values sent don't exceed max field buffers.
- **Action**: Implement a stricter sanity-check utility method before `saveConfig()` is invoked. Verify that `boards.length <= MAX_BOARDS`.
