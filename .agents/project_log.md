# Project Log

## Execution History

## 2026-03-14 - Additional Drawing Primitives

### Session Summary
Added a suite of common drawing primitives (`drawBox`, `drawLine`, `drawCircle`, `drawRoundedBox`, `drawTriangle`) to the `drawingPrimitives` helper library to simplify board-specific UI development.

### Key Decisions
- **API Harmonization**: Implemented a unified `isFilled` parameter across all applicable primitives, abstracting away the disjointed `u8g2` naming convention (`drawBox` vs `drawFrame`).
- **Triangle Extension**: Added custom line-drawing logic to `drawTriangle` to enable unfilled triangles, a feature missing from the base `u8g2` library.
- **Helper Utility Justification**: Validated the continued use of the `drawingPrimitives` library as a high-level UI abstraction layer for text centering, truncation, and screen-aware geometry.

### Git Commit
Generated commit: be78a5c

## 2026-03-14 - Weather & Config v2.1 Refactor

### Session Summary
Finalized the weather system refactor and configuration migration to Version 2.1. This included moving to an injectable, multi-location weather architecture and standardizing the configuration schema.

### Key Decisions
- **Injectable Weather**: Decided to have each `iDisplayBoard` own its own `WeatherStatus` object. This allows per-board weather tracking and refresh cycles.
- **Config v2.1**: Renamed `mode` to `type` and `boardMode` to `boardType` for better semantic clarity.
- **Automatic Migration**: Implemented logic in `ConfigManager` to detect v1.0/v2.0 configs and automatically upgrade/save them as v2.1 on first load.
- **Stateless Weather Client**: Refactored `weatherClient` to remove all internal state and simply update a provided `WeatherStatus` reference.
### Git Commit
Generated commit: 96ccdb6

## 2026-03-14 - Web Portal Refactor Phase 1: Portal Infra & Backend Refactor

### Session Summary
Established the parallel infrastructure for the modern web portal and refactored the backend web server logic. This phase enables the development of a modern SPA without disrupting the legacy web interface.

### Key Decisions
- **Parallel Evolution Strategy**: Created a dedicated `/portal` directory for modern SPA assets, allowing side-by-side operation with the legacy `/` interface.
- **Automated Asset Pipeline**: Implemented `portalBuilder.py` to gzip and hexify assets into a C++ header (`portalAssets.h`). This ensures that the frontend remains zero-dependency (no external CDNs required).
- **Decoupled Handler Manager**: Extracted portal-specific routing into a standalone `WebHandlerManager` class, using dependency injection for `WebServer` and `ConfigManager`. This improves codebase modularity and testability.
- **uiproto Standard**: Adopted the bottom navigation and sticky action patterns from the prototype, ensuring a premium "app-like" experience on embedded hardware.

## 2026-03-14 - Web Portal Refactor Phase 2: API Integration & CRUD Logic

### Session Summary
Wired the newly developed frontend prototype directly to the C++ backend. Transitioned from legacy granular POST endpoints to a modern "Unified Save" JSON API that bridges System Settings, API Keys, and Boards into a single atomistic update.

### Key Decisions
- **Unified Save (`/api/saveall`)**: Frontend now aggregates all application state into one JSON blob. This minimizes HTTP overhead and guarantees transactional-like consistency when applying config changes.
- **Dynamic SPA Rendering**: Replaced static HTML scaffolding in `index.html` with vanilla JS Template literals that dynamically fetch and render arrays (e.g., Active Boards CRUD interface, API Key list).
- **In-situ Diagnostics Polling**: The frontend now polls `/api/status` every 10 seconds, binding live RSSI, Heap size, Uptime, and IP address to the expandable drawer UI without hard-refreshing the browser.

### Git Commit
Generated commit: 642c158

## 2026-03-14 - Web UI Performance & Data Reliability Refactor

### Session Summary
Implemented a system-wide non-blocking yield mechanism to resolve 20-30s Web UI lags and fixed critical data population bugs in the Web Handlers. Also transitioned the font system to a human-readable ASCII-art source format.

### Key Decisions
- **Non-Blocking Yield Mechanism**: Introduced `yieldCallback` in all data clients (`NationalRail`, `TfL`, `Weather`, `RSS`). This allows `webServer.handleClient()` to be called during long-duration network I/O, keeping the Web interface responsive.
- **Display Manager Integration**: Centralized the yield entry point in `DisplayManager::yieldAnimationUpdate` to allow concurrent animation updates and web request handling.
- **Web Data Standardisation**: Normalized JSON keys between the legacy flat UI and modern multi-board backend (e.g. mapping `station` vs `crs`).
- **Coordinate Protection**: Implemented a "lazy update" for coordinates in `WebHandlers` to prevent zeroing out lat/lon when the browser fails to provide them during a save.
- **Font Source Format**: Moved the "Source of Truth" for fonts to human-readable `.txt` files in `modules/displayManager/fonts/source/`. This enables easier font editing and automatic BDF/binary generation.

### Git Commit
Generated commit: 383b9bd

## 2026-03-15 - Repository-wide House Style Audit & Header Migration

### Session Summary
Conducted a comprehensive audit of all project source files to ensure strict adherence to the defined house style and architectural standards. This included a major migration of project headers from `.h` to `.hpp` and the addition of exhaustive Doxygen documentation across the codebase.

### Key Decisions
- **Uhpp Migration**: Enforced the `.hpp` extension for all internal project headers to visually distinguish them from external C libraries and strictly follow the project's naming conventions (`camelCase.hpp`).
- **Header Orchestration**: Centralized module definitions and exported function lists in file headers to improve discovery and maintenance for future agents.
- **Global Variable Documentation**: Enforced same-line commenting for every global instance (e.g., `ota`, `timeManager`, `appContext`) to clarify singleton roles and initialization order.
- **Build Integrity Verification**: Prioritized a full re-build (`pio run`) after the rename to ensure that all internal and nested include paths were correctly updated and resolved.

### Git Commit
Generated commit: 15d5aa3

## 2026-03-15 - State Machine and Boot Initialization Refactor

### Session Summary
Refactored the initial boot sequence in `appContext` and `DisplayManager` to cleanly transition through states (`BOOTING`, `WIFI_SETUP`, `BOARD_SETUP`, `RUNNING`) without premature board activation or API requests under unconfigured conditions. Added a robust state management document (`AppContextStateMachine.md`).

### Key Decisions
- **`hasConfiguredBoards()` Implementation**: Modified the configuration check to evaluate `.complete` flags natively instead of blindly trusting `MAX_BOARDS` index parsing, resolving a bug where default incomplete boards bypassed the Web Setup phase.
- **Display Transition Integrity**: Fixed `appContext::tick()` to explicitly trigger `showBoard(SYS_HELP_CRS)` during the precise transition from `BOOTING` to `BOARD_SETUP`. 
- **Splash Screen Lockout**: Restructured `DisplayManager::applyConfig` to block configured boards from replacing the system `splashBoard` or `wifiWizard` until `appContext` fully transitions into the `RUNNING` state.

### Git Commit
Generated commit: 18f4bca

## 2026-03-15 - System Modules and WiFi Manager Integration

### Session Summary
Executed an architectural refactoring to strictly enforce dependency injection and eliminate global singletons. Moved `appContext` to its own namespace (`modules/appContext`) to resolve LDF scoping issues and migrated `WiFiConfig` to `modules/wifiManager`. `include/buildOptions.h` was consolidated into `src/departuresBoard.hpp` as the primary configuration header. 

### Key Decisions
- **`appContext` Module**: Isolated `appContext` to fix PlatformIO Library Dependency Finder (LDF) cyclic dependency parsing when generic libraries included `.hpp` files from the consumer `src/` directory.
- **Global `wifiManager` Removal**: Refactored `wifiManager` into a scoped member of `appContext`, transitioning all legacy handlers to retrieve it via `appContext.getWifiManager()`. 
- **House Style Compliance**: Rewrote all refactored file headers to strictly adhere to the `camelCase.hpp` and Doxygen standard. 

### Git Commit
Generated commit: 0c8551e

## 2026-03-15 - Web Portal Refinement: WiFi Reset & Connectivity Stability

### Session Summary
Implemented the "WiFi Reset" feature to allow users to erase credentials and reboot the board from the UI. Addressed critical connectivity issues, including a password mask mismatch and redundant WiFi reconnection logic. Performed a major House Style refactor to standardize library naming and documentation.

### Key Decisions
- **WiFi Reset Logic**: Implemented `handleWiFiReset` in `webHandlerManager` to call `wifiManager.resetSettings()`, respond to the client, and then trigger `ESP.restart()` after a delay, ensuring the user receives feedback before the board drops the link.
- **Connection Test Optimization**: Modified `testConnection` to skip `WiFi.begin()` if the requested credentials match the current active connection. This prevents the browser session from dropping during a redundant test, providing a seamless user experience.
- **House Style Library Renames**: Renamed `WiFiConfig`, `Logger`, and `HTTPUpdateGitHub` libraries to `camelCase` (`wiFiConfig`, `logger`, `hTTPUpdateGitHub`) to align with project standards.
- **LDF Discovery Fix**: Added `library.json` files to several `modules` to help PlatformIO's Library Dependency Finder correctly resolve nested dependencies between shared logic and system management.

### Git Commit
Generated commit: 5eb9aee

## 2026-03-15 - Web Portal Test Suite & API Key Verification

### Session Summary
Implemented a comprehensive testing suite (Playwright for E2E web, Unity for C++) to ensure portal reliability. Resolved a critical portal unresponsiveness bug caused by minification corruption and missing JS methods. Conducted an architectural review verifying the encapsulation of the API Key Registry based on embedded systems best practices.

### Key Decisions
- **Two-Pronged Testing**: Separated web tests into "Local Mocked Tests" (for logic and destructive actions) and "Live Hardware Tests" (for smoke testing against the actual ESP32).
- **Build Integration**: Added a pre-build Python python script (`run_web_tests.py`) to the PlatformIO toolchain to automatically run mocked UI tests before compiling firmware, preventing regressions from reaching hardware.
- **Minifier Tightening**: Updated `portalBuilder.py` to aggressively strip single-line comments and whitespace, reducing the embedded HTML asset from 31KB to 19KB.
- **Architectural Validation**: Confirmed that the API Key components correctly use abstraction (referencing `apiKeyId` rather than the text key), and that secrets remain isolated in `/apikeys.json` while being redacted from serial logging.

### Git Commit
Generated commit: da2252b

## 2026-03-15 - WiFi Portal Usability & Build Automation

### Session Summary
Resolved critical WiFi portal usability issues and race conditions. Improved UI alignment and pre-selection logic for the configured network. Integrated portal UI assets into the automated PlatformIO build pipeline to ensure frontend changes are reliably reflected in firmware.

### Key Decisions
- **Asynchronous Initialization**: Refactored the portal's `init()` function to `await` configuration data before rendering. This eliminates the race condition where empty scan results would overwrite the pre-selected configured SSID.
- **Configured Network Priority**: Implemented logic to always place the currently configured SSID at the top of the list, labeled with a lightning bolt (⚡) and "(Configured)", providing instant feedback in Access Point mode.
- **UI Alignment & Stability**: Fixed misalignment of the 'Scan' button and password 'eye' icon. Stabilized button widths to prevent layout shifts during active scanning.
- **Automated Asset Synchronization**: Integrated `portalBuilder.py` as a pre-build script in `platformio.ini`. This automates the minification and embedding of `/portal/index.html` into the C++ header file, preventing manual synchronization errors.
- **House Style Audit**: Updated headers for `portal/index.html` and the generated `portalAssets.h` to include full "Exported Functions/Classes" lists, satisfying project documentation standards.

### Git Commit
Generated commit: 8e61ab9

## Bug Report - Agent Session "Mega Crash"

### Date
2026-03-15T13:40Z (Estimated based on queue log)

### Description
The Antigravity agent session tasked with "Implement Keys Manager Registry (CRUD, Key IDs, Board Mapping)" crashed mid-execution. The agent's conversation (`abb31206-f8fc-443b-a9ed-4c604e1a4de6`) was abruptly terminated, resulting in a loss of session state and chat history. A stale lock was left in `.agents/queue.md`. 
Upon investigation, there was no internal git crash checkpoint stored in `.gemini/antigravity/code_tracker/history` for today's session. However, the exact state of the refactoring was still present in the user's working tree as 20 distinct uncommitted file modifications (spanning `index.html`, `configManager`, board implementations, etc.). These changes have been successfully captured and safely stored in the `temp/recovery-api-key-refactor` branch.

### Steps to Reproduce
1. Start an agent session with a complex, multi-file refactoring task (e.g., modifying `index.html` bindings and multiple C++ `configManager` references concurrently).
2. Allow the session to perform extensive uncommitted file modifications in the working tree.
3. Observe the system prematurely crashing; the chat context and session state is lost, leaving a stale lock in `.agents/queue.md`, while the workspace retains the uncommitted modified files.

## 2026-03-16 - API Key UI Refinement & Build Pipeline Fix

### Session Summary
Refined the API Key Registry UI for better layout stability and added branded styling. Resolved a critical build synchronization issue where frontend changes were not being correctly reflected on the hardware. Improved WiFi connectivity status indicators and removed the legacy `bustimes.org` provider.

### Key Decisions
- **Stable Modal Layout**: Introduced `.modal-button-row` to enforce equal-width, rigid buttons (Test, Cancel, Save). Removed the spinner from the 'Test' button to prevent layout shifts and text jumping during the "Testing..." state.
- **Relocated Delete Action**: Moved the "Delete Key" button to the bottom of the modal as a full-width element with distinct vertical spacing (2rem) and added a confirmation dialog.
- **Build Integration Fix**: Discovered that PlatformIO was not reliably recompiling the web handlers after asset updates. Integrated `portalBuilder.py` with timestamp checks and verified that clean builds/forced object file removal ensures hardware sync.
- **WiFi Status Logic**: Refined the frontend status indicator to show a red dot for disconnected states, yellow for low RSSI, and green for healthy links, with matching text labels.
- **Provider Cleanup**: Removed the `bustimes.org` scraper provider from the key selection dialog and updated the Playwright test suite to match.

### Git Commit
Generated commit: ef23372

## 2026-03-16 - Portal UI Polish: Eye Icons, NR & TfL Logos

### Session Summary
Corrected the inverted logic for the password visibility toggle and upgraded the National Rail and TfL provider logos to their official SVG versions. Verified the fixes on live hardware across multiple deployment iterations. Addressed initial stale-code issues through clean builds and forced browser cache bypass.

### Key Decisions
- **CSS-Driven Toggle**: Transitioned the password visibility logic to rely on the CSS `active` class. This resolved conflicts with `!important` rules in the dark-mode CSS and ensured hardware-consistent performance.
- **Icon Logic Harmonization**: Standardized the visibility icon states to use the "Slashed Eye" for the visible password state and the "Open Eye" for the masked state, matching modern digital accessibility patterns.
- **Provider Logo Refresh**: Replaced outdated logos in the `PROVIDER_LOGOS` registry and selection modals with high-resolution, branded SVGs (National Rail double-arrow and TfL Roundel with `#0019A8` fill).
- **Automated Validation Integration**: Verified all UI changes using both local simulation and live hardware subagent testing, ensuring that user-provided formatting changes did not introduce regressions.

### Git Commit

## 2026-03-16 - Repository-wide House Style Update: Refactoring Attribution

### Session Summary
Applied the new refactoring attribution line to the standard module header across the entire C++ codebase (87 files). Optimized the build system configuration to prioritize hardware targets and renamed the host-side test environment to `unit_testing_host`. Enhanced the agent skills infrastructure by adding targeted triggers to the `house-style-documentation` skill and migrating the project TODO list to the `.agents/` directory.

### Key Decisions
- **Automated Replacement**: Used `sed` with a repository-wide `find` search to ensure 100% consistency across all `.cpp` and `.hpp` files.
- **Documentation Alignment**: Synchronized the project's "House Style" definition and added explicit triggers (`review-ip`, `ip-review`) to the agent skills to ensure consistent policy enforcement.
- **Build Optimization**: Configured `default_envs` in `platformio.ini` and renamed the native environment to `unit_testing_host`. This provides semantic clarity and prevents hardware-dependency build errors when running standard compilation commands.
- **Infrastructure Consolidation**: Migrated legacy `TODO.md` to `.agents/todo_list.md` and removed obsolete `.github` rules to centralize all agent logic.

### Git Commit
Generated commits: 10a9bbf, b185905, 011bd9f

## 2026-03-16 - Sequential API Key Validation & Memory Optimization

### Session Summary
Implemented sequential API key validation in the web portal to prevent ESP32 network module saturation and handled stack overflow issues by offloading SSL clients to the heap. Improved visual feedback for API tests and network disconnection states.

### Key Decisions
- **Sequential Validation Logic**: Orchestrated API key tests to execute one-by-one with a 1-second delay. This ensures hardware stability during the intensive SSL handshake process and provides a predictable user experience with sequential status dot updates.
- **Heap-Allocated SSL Clients**: Refactored `NationalRailDataSource` and `TfLDataSource` to use heap-allocated `WiFiClientSecure` objects. This prevents stack overflow crashes during complex TLS handshakes without introducing long-term fragmentation (short-lived allocations).
- **Network Error Observability**: Integrated a `isWifiPersistentError` method and updated `wifiStatusWidget` with a blinking logic (after 30s) to clearly signal prolonged connectivity loss on the hardware display.
- **Safe Password/Key Masking**: Implemented a "No Change" detection pattern for password and API key fields. The system now ignores placeholders (`••••••••`) during saves, preventing accidental erasure of existing secrets.

### Git Commit

## 2026-03-17 - Feeds Tab Stabilization & Async Diagnostics

### Session Summary
Finalized the FEEDS tab UI and back-end diagnostic logic. Implemented asynchronous auto-testing for News and Weather feeds, stabilized "Test Feed" buttons at 44px height, and added a grey "UNCONFIGURED" state for deselected weather keys. Migrated the RSS feed list to a separate `rss.json` file to simplify management and decoupling from the main HTML.

### Key Decisions
- **Button Stabilization**: Applied fixed `height: 44px` and zero vertical padding to `.btn-test-action` to eliminate layout shifts during the "Testing..." state.
- **RSS JSON Migration**: Moved the RSS feed list to a separate `rss.json` asset served via the new `handleRSSJson()` endpoint, allowing for easier feed updates without re-building the entire portal.
- **Asynchronous Auto-Testing**: Configured the FEEDS tab to trigger background connectivity tests immediately upon opening, allowing diagnostic dots to populate without stalling the UI.
- **Multi-Key Weather Logic**: Enhanced `weatherClient` to support specific key IDs and override tokens, enabling real-time validation of unsaved keys in the portal.

### Git Commit
Generated commit: 6896809

## 2026-03-17 - Portal Navigation Reorder & Build Script Fix

### Session Summary
Reordered the bottom navigation bar of the web portal to follow the user flow (WiFi, Keys, Feeds, Displays, Schedule, System). Added a placeholder for the "Schedule" feature and fixed a build script integration issue that was preventing the portal assets from being correctly embedded during compilation.

### Key Decisions
- **User Flow Reorder**: Moved the navigation links and their corresponding content sections in `index.html` to align with the logical setup sequence requested.
- **Schedule Tab Placeholder**: Added a new "Schedule & Auto-Power" tab with a "Feature under development" message and icon, providing a UI hook for future development.
- **`portalBuilder.py` Integration**: Corrected a logic error in the build script that only ran the asset generation if the script was executed as `__main__`. This fix ensures it runs correctly when invoked by PlatformIO as a `pre:` script, guaranteeing the latest portal changes are included in every firmware build.
- **Serial Port Cleanup**: Implemented aggressive process termination for stale `pio` and `esptool` instances to resolve hardware upload conflicts.

### Git Commit
Generated commit: ba084c4

## 2026-03-17 - Unified Test Queue & Display UI Unification

### Session Summary
Implemented a centralized `app.state.testQueue` in the portal to serialize all diagnostic requests (Boards, Keys, Feeds, Weather). This prevents the ESP32 from being overwhelmed by concurrent SSL/API requests. Unified the diagnostic UI for the "Displays" tab to match the "API Keys" style, adding explicit status text labels.

### Key Decisions
- **Serialized Diagnostic Queue**: All diagnostic tests now run sequentially with a 500ms breather, resolving the "HTTP ERROR" and "Red Dot" failures caused by socket exhaustion.
- **Tab Context Sensitivity**: Integrated `clearTestQueue` into the tab switcher to immediately cancel stale tests from previous views.
- **Visual Feedback Unification**: Added status text labels ("Testing...", "Active", "Offline") to board slots, improving UX and consistency across system diagnostic views.
- **Race Condition Resolution**: Moved health check triggers to execute after the DOM is fully rendered in `renderBoards` to ensure target element availability.

### Git Commit
Generated commit: 175be62

## 2026-03-17 - Unified iDataSourceTest Interface & API Key UI Refactoring

### Session Summary
Standardized the way external data sources verify their connectivity and authentication credentials by introducing a `testConnection` method to the `iDataSource` interface. Refactored `WebHandlerManager` to encapsulate and delegate testing logic directly to the individual data source implementations.

### Key Decisions
- **Interface Segregation**: Added `virtual int testConnection(const char* token = nullptr)` to `iDataSource`, making data sources responsible for their own internal test capabilities rather than hardcoding tests in the web handler.
- **National Rail Optimization**: Used the lightweight V12 payload and the default "PAD" CRS code to quickly validate the National Rail token without pulling the 500KB WSDL.
- **TfL Validation**: Implemented token validation for TfL using the `/Line/victoria/Status` endpoint as a lightweight test vector.
- **Bus Non-Auth Path**: Allowed `busDataSource` to bypass authentication checks by immediately returning success, as bustimes.org does not currently require tokens.
- **House Style Assurance**: Guaranteed documentation standards by appending mandatory Doxygen headers for all newly created methods.

### Git Commit
Generated commit: 7573fe2

## 2026-03-18 - Display Lifecycle Telemetry & Architecture Refactor (Session deb59b03)

### Session Summary
Audited the `DisplayManager` and `systemManager` coupling to ensure robust transition logic. Decoupled internal system handlers from explicit board classes by implementing the Observer Pattern in the `appContext` orchestrator. Enhanced the unified diagnostic logging pipeline to track exhaustive UI footprint events across all display modalities.

### Key Decisions
- **Orchestration Mediator**: Abstracted UI dependencies away from `systemManager` and `otaUpdater` using `std::function` callbacks. These callbacks are dynamically assigned within `appContext::begin()`.
- **Semantic UI Telemetry**: Added a required `getBoardName()` string implementation to the `iDisplayBoard` contract, forcing all 11 display boards to self-identify.
- **Contextual View Logs**: Extended `DisplayManager::showBoard(board, reason)` to accept and log the exact `reason` the viewing context changed (e.g., config save, carousel rotate, OTA interrupt).
- **Compilation Validation**: Confirmed syntactical soundness and verified that zero overhead/errors were introduced to the `esp32dev` toolchain.

**Archive**: [.agents/plans/done/deb59b03-cb96-4a9c-afff-69421abb7936/](.agents/plans/done/deb59b03-cb96-4a9c-afff-69421abb7936/)

### Git Commit
Generated commit: [TBD - Captured in active workspace]

## 2026-03-18 - FreeRTOS Asynchronous Data Fetching Refactor

### Session Summary
Addressed system blocking and WDT (Watchdog Timer) resets on ESP32 single-core models by refactoring `weatherClient`, `rssClient`, `busDataSource`, `tflDataSource`, and `nationalRailDataSource` to use asynchronous FreeRTOS background tasks. Created `AsyncDataRetrieval.md` as standard core architecture documentation.

### Key Decisions
- **FreeRTOS Task Dispatching**: Offloaded HTTP requests and JSON/XML streaming parses to Core 0 via `xTaskCreatePinnedToCore`, yielding `UPD_PENDING` instantly so the UI rendering thread remains unblocked.
- **Double Buffering & Synchronization**: Implemented background structs (`stationData`, `bgStatus`) alongside UI-safe buffers (`renderData`, `activeStatus`), strictly protected via `SemaphoreHandle_t` during state swaps.
- **Single-Core Priority Yielding**: Added deterministic `vTaskDelay(1)` yielding intervals (e.g., every 500 JSON loops) into processing cycles. This guarantees WiFi hardware interrupts achieve scheduling priority, eliminating TWDT panics on ESP32-C3/S3 single-core chips.
- **Comprehensive Docs**: Verified all new headers and thread-safe member variables comply with Doxygen House Style.

### Git Commit
Generated commit: 8585fff

### 2026-03-17: TimeManager Strict DI Refactor (cceae27)
- **Goal**: Refactor the global `timeinfo` structure and NTP synchronization into a fully encapsulated module.
- **Key Decisions**: `TimeManager` is now injected centrally via `appContext`, enabling isolated time testing and preventing any race conditions from detached background time updates on floating struct globals.
- **Outcomes**: Removed all `extern struct tm timeinfo` directives from UI components. Added comprehensive Doxygen documentation for all DI interfaces in `appContext` and displaying widgets per House Style. Added isolated TimeManager testing to the TODO list.

### 2026-03-17: Archiving Legacy Web Infrastructure (c5b9ef6)

### Session Summary
Archived the legacy web page infrastructure to an `archive` folder, removing them from the main compilation process. Renamed the modern portal from `portal/` to `web/`. Consolidated versioning across C++ and web layers.

### Key Decisions
- **Archive Legacy Assets**: Moved `web/` to `archive/web/` and `include/webgui/` to `archive/include/webgui/`.
- **Portal Rename**: Renamed the modern SPA directory from `portal/` to `web/` and updated all C++ routing to serve from `/web`.
- **Unified Versioning**: Removed `WEBAPPVER_MAJOR` and `WEBAPPVER_MINOR` in favor of a unified `VERSION_MAJOR` and `VERSION_MINOR` in `departuresBoard.hpp`.
- **Cleaned Up Server Logic**: Removed legacy `handleNotFound` and unused route registrations from `webServer.cpp`, and added a 302 redirect from `/` to `/web`.

### Git Commit
Generated commit: e727ef3

## 2026-03-18 - Screensaver Clock Enhancements (Session 685a3ce)

### Session Summary
Enhanced the Screensaver Clock configuration and the overall portal aesthetic. Implemented per-board brightness overrides, refined the board modal to hide redundant fields for clocks, and updated the diagnostics system to show real-time device time and timezone.

### Key Decisions
- **Orange Sliders**: Applied `accent-color: var(--primary)` globally to range inputs to match the brand's orange color.
- **Clock Diagnostics Override**: Replaced Latitude/Longitude with Local Time and Timezone in the diagnostics drawer specifically for Clock boards.
- **Per-Board Brightness**: Added an `int brightness` field to `BoardConfig` (defaulting to -1 for global sync) to allow per-board display levels.
- **Enhanced Status API**: Updated `/api/status` to return the formatted local time and timezone from the ESP32's `TimeManager`.

**Archive**: [.agents/plans/done/685a3ce5-c3cd-41b8-a78a-3eea17b2e153/](.agents/plans/done/685a3ce5-c3cd-41b8-a78a-3eea17b2e153/)

### Git Commit
Generated commit: 6acd617

## 2026-03-18: System Tab Enhancements (Session 7556e82)
- **Goal**: Consolidate global settings and implement dynamic diagnostics.
- **Accomplishments**: 
    - Reorganized System tab with progressive hardware bars (Heap, Storage, Temp, Uptime).
    - Optimized polling with Visibility API support and 5s active interval.
    - Implemented 2-step factory reset and backend handlers for reboot/OTA.
    - Relocated Hostname to WiFi tab for network discovery priority.
- **Key Decisions**: Used Vanilla JS for diagnostics and 5s polling to balance responsiveness with ESP32 overhead.
- **Commit**: 7556e82
 
 ## 2026-03-18 - Dynamic Default Tab Selection (Session 16b784a)
 
 ### Session Summary
 Implemented "smart" initial tab selection for the web portal. The application now assesses device connectivity, API keys, and board configuration on load to guide the user to the most relevant tab (WiFi, Keys, or Displays).
 
 ### Key Decisions
 - **One-Time Selection**: Used an `initialTabSet` flag to ensure the automatic logic only runs once on load, preventing background status polls from overriding manual user navigation.
 - **Priority Logic**: WiFi takes ultimate priority (if disconnected/AP mode), followed by Keys (if setup is missing), defaulting to Displays for existing configurations.
 - **Deferred AP Mode Switch**: Updated `updateStatusUI` to only force the WiFi tab if a connection loss occurs *after* the initial portal load.
 - **House Style Refinement**: Added Doxygen-style documentation to the new frontend logic to maintain consistency with the C++ codebase.

**Archive**: [.agents/plans/done/16b784aa-e89e-432b-a85a-ff992f95c8dc/](.agents/plans/done/16b784aa-e89e-432b-a85a-ff992f95c8dc/)

### Git Commit
 Generated commit: 2780ca9
 
 ### Build Statistics
 - **RAM Usage**: 24.3% (79.7 KB)
 - **Flash Usage**: 69.9% (1.37 MB)

## 2026-03-19 - Centralized Data Worker Queue Refactor (Session 206c4ba2)

### Session Summary
Addressed heap fragmentation and OOM panics in `WiFiClientSecure` caused by concurrent multi-board data polling. Implemented a centralized `dataWorker` queue on Core 0 to serialize external API requests across all transport modules (`weather` and `nationalRail/tfl/bus`). 

### Key Decisions
- **Serialized Worker Task**: Created a singleton FreeRTOS queue (`dataWorker.hpp`) to enforce one-at-a-time network TLS handshakes, drastically reducing peak memory usage without sacrificing UI rendering thread availability.
- **iDataSource Contract Upgrade**: Promoted `executeFetch()` to the public `iDataSource` abstraction, stripping autonomous `xTaskCreatePinnedToCore` logic out of individual client modules.
- **Deduplication Blocking**: Standardized `UPD_PENDING` tracking in local source modules to prevent duplicate task queued submissions when a network request is already processing.

**Archive**: [.agents/plans/done/206c4ba2-972f-4741-96a6-a4b918e935de/](.agents/plans/done/206c4ba2-972f-4741-96a6-a4b918e935de/)

### Git Commit
Generated commit: 8a58e10

## 2026-03-22 - LGV Terminology Refactor & Documentation Audit (Session 1cf75709)

### Session Summary
Systematically renamed all instances of "View" to "Layout" across the core display architecture and architecture documents (LGL instead of LGV) to align with the "Display Designer" conceptual model. Conducted a comprehensive documentation audit across the entire `modules/displayManager` directory to standardize module headers and add Doxygen comments to all widget and board controllers.

### Key Decisions
- **Terminology Alignment**: Migrated from the generic `View` term to `Layout` across C++ code and Markdown documentation to better describe the visual arrangement of widgets on the board.
- **Physical Structure Renaming**: Renamed all `views/` directories to `layouts/` and updated corresponding include paths.
- **Contract Standardization**: Renamed `iBoardView` to `iBoardLayout` and all derived board-specific interfaces (e.g., `iNationalRailView` -> `iNationalRailLayout`).
- **House Style Assurance**: Guaranteed documentation standards by appending mandatory Doxygen headers for all `displayManager` methods and ensuring module descriptions.
- **Hardware Verification**: Successfully flashed and verified on ESP32, confirming stable board initialization and data rendering under the new terminology.

**Archive**: [.agents/plans/done/1cf75709-e370-401c-a1f6-a8d71ba9fefe/](.agents/plans/done/1cf75709-e370-401c-a1f6-a8d71ba9fefe/)

### Git Commit
Generated commits: 418144a (Refactor Code), c076120 (Documentation & House Style)

## 2026-03-22 - WebServer Refactoring & Multi-Board Background Sync (Session 470d6a16)

### Session Summary
Successfully migrated the legacy blocking `WebServer` logic to completely asynchronous handling using `ESPAsyncWebServer`, decoupling the core API pulling logic and unblocking ESP32 thread availability. Implemented a robust 'Round-Robin' API polling distribution matrix natively into the central `systemManager` object hierarchy, resolving long-standing issues concerning background connectivity caching and data synchronization throttling for National Rail, TfL, and Bus displays.

### Key Decisions
- **Async Diagnostic Payloads**: Refactored chunking memory payload buffers `_tempObject` to dynamically protect explicit verification queries targeting the API Key components.
- **Geographic Data Validation**: Re-integrated the upstream `/stationpicker` handler gracefully to reconstruct missing geospatial tracking coordinates for NRE stations.
- **API Distribution Array**: Implemented the mathematical `(config.apiRefreshRate / config.boardCount)` interval loop to gracefully execute non-blocking, parallel background board synchronization queues perfectly avoiding upstream developer rate limits.
- **Fast-Fill Initialization**: Instantiated a priority 'Fast-Fill' interval state (`2000ms`) prioritizing aggressive queueing of offline hardware configurations (`lastUpdateStatus == -1`) universally at boot.
- **Dynamic UX Feedback**: Overrode static drawing elements throughout transport layout structures with real-time `Loading data...` status overlays when boards accurately resolve back to `UPD_PENDING` (`9`) execution modes.

**Archive**: [.agents/plans/done/470d6a16-dd63-42c0-bc27-62a58dac6dd2/](.agents/plans/done/470d6a16-dd63-42c0-bc27-62a58dac6dd2/)

### Git Commit
Generated commit: 64db430

## 2026-03-22 - labelWidget UI Component Architecture (Session 3ab2c4b6)

### Session Summary
Implemented a new graphical component, `labelWidget`, to wrap bare-metal `u8g2` drawing primitives in an object-oriented state. This abstraction was subsequently deployed across all Transport-class Boards (`TfL`, `Bus`, `NationalRail`) by introducing an architectural pattern of "Visibility Toggling" during missing-data states rather than dynamic imperative drawing routines.

### Key Decisions
- **Designer Preparedness**: Implemented `labelWidget` inheriting `iGfxWidget` and explicitly tagged `setFont`, `setAlignment`, and `setTruncated` setters with `@designer_prop` to guarantee forwards compatibility with the upcoming visual drag-and-drop Display Designer tool.
- **Embedded SRAM Policy**: Intentionally excluded the static "System Boards" (`splashBoard`, `loadingBoard`, `messageBoard`, etc.) from adopting `labelWidget`. This preserves roughly 140 bytes of permanent SRAM per string that would otherwise be wasted encapsulating strings that never dynamically update or move. Documented this deliberate non-migration within `docs/SystemSpecificationDocument.md`.
- **Visibility-Driven Layouts**: The empty data fallback behavior ("No scheduled services.") on Transport Boards was migrated directly into the base Layout classes (`iTflLayout`, `iBusLayout`, etc.) using `labelWidget` objects that are toggled on and off via `.setVisible()`.
- **Validation**: Performed sequential `esp32dev` and `esp32s3nano` PlatformIO builds to ensure full native compilation compatibility and no static memory bloat.

### Git Commit
Generated commit: 9f6445e

## 2026-03-23 - Thread-Safe Reconfiguration & Boot Stability (Session eaf7823b)

### Session Summary
Diagnosed and resolved a fatal `LoadProhibited` race condition occurring during configuration reloads. Implemented a deferred synchronization pattern in `ConfigManager` and `appContext` to ensure layout memory is only manipulated by the primary rendering core. Optimized National Rail initialization by implementing a standalone `/darwin_wsdl_cache.json` system that bypasses legacy WSDL fetches while maintaining dynamic discovery and self-healing.

### Key Decisions
- **Deferred Reconfiguration**: Introduced an atomic `reloadPending` flag to decouple the `AsyncWebServer` network thread from the `DisplayManager` layout lifecycle, preventing memory access violations during I2C draw cycles.
- **Continuous status Polling**: Fixed a "Loading..." UI deadlock by allowing the round-robin controller to continuously poll `updateData()` for pending boards, ensuring display widgets can read completion states from the background `DataWorker`.
- **Standalone WSDL Cache**: Migrated the National Rail SOAP discovery results from `config.json` to a dedicated `/darwin_wsdl_cache.json` file. This preserves configuration purity while achieving millisecond boot times for the transport data source.
- **Dynamic Discovery & Self-Healing**: Configured the system to perform a live WSDL fetch only if the cache is missing or if the current endpoint returns a 404/5xx error, ensuring long-term API adaptability without daily performance penalties.
- **Sequential Test Routing**: Enforced that all web portal API validation tests must be enqueued through the `DataWorker` to prevent concurrent TLS allocation crashes.

### Git Commit
Generated commit: [TBD]

## 2026-03-26 - Display Output Robustness and Binding Data Paths (Session 453f32fb)

### Session Summary
Systematically rooted out and resolved visual leakage and missing memory data injection issues rendering artifacts across the display pipeline. Resolved the broken clipping masks by implementing 8-bit wrap-around safeguards and forced strict display state encapsulation using a new `U8g2StateSaver` RAII pattern. Synchronized the WebAssembly simulator mock data mapping logic to mirror physical hardware extraction.

### Key Decisions
- **RAII Display Isolator**: Forced all `iGfxWidget` subclasses to wrap hardware drawing interactions utilizing `U8g2StateSaver`. This unconditionally prevents cascading memory failures (like incorrect font sizes and dirty clipping bounds) when layout hierarchies are deeply traversed.
- **Darwin Buffer Remediation**: Identified that the National Rail XML parsing pipeline was misassigned to `location` while the rendering layer mapped to an empty `stationName` array. Upgraded the `nationalRailBoard` controller to bypass the discrepancy directly instead of introducing generic mirroring buffers.
- **Boundary Mask Wrap Limits**: Repaired the `setClipWindow` primitive across all scrolling data panels by constraining right-side edges to `255px`. Previously, subtracting offset from `256` flipped the 8-bit unsigned integer to `0`, fully deleting the masking geometry during scroll.
- **WASM Payload Accuracy**: Updated `mockDataManager` handling in the physical simulator layout to match the correct internal XML layout formats.

**Archive**: [.agents/plans/done/453f32fb-912c-442c-8add-c6faf299ad89/](.agents/plans/done/453f32fb-912c-442c-8add-c6faf299ad89/)

### Git Commit
Generated commit: 06d0b2c

## 2026-03-27 - Clock Widget Real-Time Update & Colon Blink Fix (Session d986faf3)

### Session Summary
Diagnosed and resolved a "stuck clock" issue where the `clockWidget` on standard departure boards was only updating during network fetch yields. Implemented a forced `updateCurrentTime()` poll inside the widget's `render()` method to ensure real-time accuracy during the main 60Hz display loop. Simultaneously repaired the colon blinking state machine by correctly tracking `oldColon` transitions. Verified the fix using the `flash-test` protocol on physical `esp32dev` hardware.

### Key Decisions
- **Main Loop Synchronization**: Moved the primary `updateCurrentTime()` trigger into the widget's `render()` pass. This ensures the clock remains accurate without requiring a dedicated background timer or relying on intermittent network yields.
- **State Tracking Correction**: Fixed the logical error in `renderAnimationUpdate()` where `oldColon` was never updated, which previously caused the colon to either stay static or blink erratically depending on the initial state.
- **House Style Alignment**: Audited and updated the `clockWidget.cpp` module header and documentation to strictly adhere to v3.0 project standards.

### Git Commit
Generated commit: 5abd5f3 (Fix: Real-time clock updates and colon blinking in clockWidget)

## 2026-03-27 - OLED Sleep Configuration Refactor (v2.4) (Session cf3f6db7)

### Session Summary
Migrated the \"Turn OLED Off completely in sleep\" setting from a global system configuration to a per-board configuration within the Screensaver (`SleepingBoard`) module. Implemented a robust JSON migration path (v2.3 to v2.4) and state-tracked power management in `DisplayManager` to prevent \"stuck off\" states during board transitions. Verified the implementation through hardware flashing, serial log analysis, and Playwright E2E tests.

### Key Decisions
- **Per-Board Privacy**: Finalized the decision to move OLED power management into `BoardConfig`. This allows users to have multiple clocks with different power behaviors (e.g., a "Clock" that stays on and a "Sleep" board that turns off).
- **DisplayManager Safety**: Implemented a mandatory `setPowerSave(false)` call in `DisplayManager::showBoard()`. This acts as a centralized safety mechanism that wakes the hardware before any new board is rendered, preventing logical state leakage from the previous board.
- **State Tracking**: Added `oledPowerSaveActive` to `DisplayManager` to eliminate redundant I2C/SPI transactions to the SSD1322 controller when the power state hasn't changed.
- **v2.4 Migration Engine**: Added a targeted migration block in `ConfigManager` that extracts the legacy global `turnOffOledInSleep` and applies it to the first `MODE_CLOCK` board found, ensuring a seamless OTA upgrade for v2.3 users.
- **Web UI Interaction Fix**: Resolved a JavaScript crash in the board editor where `form.elements` mapping failed for certain board types. Optimized the DOM access to use robust string-based lookups for the `oledOff` checkbox.

**Archive**: [.agents/plans/done/cf3f6db7-7ac3-4267-8341-fccc4937cf71/](.agents/plans/done/cf3f6db7-7ac3-4267-8341-fccc4937cf71/)

### Git Commit
Generated commit: 131c79c (Refactor: Migrate OLED Power Management to Per-Board Configuration)

## 2026-03-27 - Unifying DataManager Dispatch & Thread Safety (Session 56c5ef3d)

### Session Summary
Finalized the stabilization of the new `DataManager` architecture by diagnosing intermittent multi-thread hardware crashes (`LoadProhibited`) resulting from unsynchronized memory mapping between the Core 0 network fetcher and Core 1 OLED render matrices. Completed full 5-minute physical hardware validation displaying zero missed frames and perfect Priority Queue interrupts for live Web Portal E2E API tests.

### Key Decisions
- **RAII Mutex Envelopes**: Eliminated visual tearing and `LoadProhibited` segfaults by implementing explicit `lockData()` and `unlockData()` interfaces inside `iDataSource` using FreeRTOS `SemaphoreHandle_t`. The Core 0 Double-Buffer performs instant microsecond copies while Core 1 locks across its 30ms hardware rendering cycle.
- **Dual-Dispatch Unification**: Corrected a deadlock occurring within `webServer` where transient `ApiTestDataSource` objects were dropped by the queue. Consolidated the `workerTaskLoop` logic so predictive polling and sudden hardware interruptions resolve through a single, strict `executeFetch()` dispatch block.
- **Predictive Backoff Net**: Forced a global 15-second backoff in `DataManager` strictly guaranteeing that data handlers returning early on `HTTP 401/403` do not infinite-loop the core parser interval. 
- **Diagnostics Extension**: Injected real-time Free Heap mapping and internal SoC thermals out bounding across the `departuresBoard::loop` Heartbeat.

**Archive**: [.agents/plans/done/56c5ef3d-6d3b-418a-ac5e-323d7e6b7226/](.agents/plans/done/56c5ef3d-6d3b-418a-ac5e-323d7e6b7226/)

### Git Commit
Generated commit: c2d975f

## 2026-03-28 - Architecture Refactoring, Web Stability & Dynamic Layouts

### Session Summary
Successfully implemented Dynamic Layout Selection allowing per-board assignment of distinct visual templates (e.g., "Default" vs "Replica") directly via the Web Portal. Stabilized system operations by diagnosing and mitigating `Task Watchdog Timer (TWDT)` crashes inside the HTTP pipeline, and repaired weather condition synchronization to prevent premature network backoffs during the boot cycle. Included custom compiled aesthetic upgrades to U8G2 weather and WiFi fonts.

### Key Decisions
- **Dynamic Board Layouts**: Expanded the `BoardConfig` schema across `configManager` to securely persist the `layout` property. Updated the frontend `index.html` UI with a new dropdown selector and ensured symmetric configuration via `handleSaveAll()` and `handleGetConfig()`.
- **WDT Polling Immunity**: Integrated explicit `esp_task_wdt_reset()` checks into the synchronized Web Server diagnostic loops (`handleTestBoard`, `handleTestKey`), fundamentally repairing the `async_tcp` (Core 1) panics occurring during SSL handshake timeouts.
- **Boot Sequence Validation**: Fixed a debilitating race condition inside `weatherClient` where incomplete `WeatherStatus` properties triggered forced 15-second data polling backoffs.
- **Custom Font Recompilation**: Seamlessly integrated user modifications to ASCII source blocks (`WeatherIcons11.txt`, `WifiIcons11.txt`) into immutable `fonts.cpp` binaries via `build_fonts.py`.

### Git Commit
Generated commit: beb71e7

## 2026-03-28 - Boot Sequence Optimization & Fetch Guarding (Session bb169fad)

### Session Summary
Optimized the ESP32 firmware boot sequence to reduce perceived latency and eliminate redundant network traffic. Implemented strict state-driven guards in `DataManager` and source clients (`weatherClient`, `rssClient`) to prevent premature API fetches during the `BOOTING` phase. Resolved a significant regression in the startup animation and UI responsiveness caused by scheduler interference during the NTP sync block.

### Key Decisions
- **Stable-State Initialization**: Deferred non-critical configuration notifications (like `resumeDisplays`) until the system reaches the `RUNNING` state, ensuring the OLED stays locked to the `LoadingBoard` during critical connect cycles.
- **Fetch Site Guarding**: Implemented "silent guards" directly inside `executeFetch()` for all data sources. This acts as a final firewall against unconfigured or early-state network requests, regardless of who invokes the update.
- **Scheduler Suspension**: Wrap the `DisplayManager` adaptive schedule and carousel rotation in an `AppState::RUNNING` guard, preventing the display from "flickering" or swapping boards while system services are still initializing.
- **Smooth Animation Interpolation**: Restored fluid progress bar movement by explicitly providing a `500ms` animation duration to `setProgress()` calls during the blocked clock synchronization phase.

**Archive**: [.agents/plans/done/bb169fad-7f19-44b1-b951-c88e17d0fdad/](.agents/plans/done/bb169fad-7f19-44b1-b951-c88e17d0fdad/)

### Git Commit
Generated commit: e21917d

## 2026-03-28 - Clock Widget Extension & WASM Schema Validation (Session 2c2d6708)

### Session Summary
Implemented support for the `HH_MM_SS` format and custom secondary font allocations in the `clockWidget`. Architected a seamless C++ to WASM registry mapping mechanism that enables the layout simulator to introspect C++ types. 

### Key Decisions
- **U8g2 Baseline Alignment**: Implemented font-safe `getAscent()` arithmetic to vertically match the seconds digits directly to the primary hours/minutes baseline, avoiding hardcoded coordinate offsets.
- **Secondary Font Hydration**: Decoupled the structural representation of the widget's "seconds" from its font size, enabling layouts to specify `"format": "HH_MM_SS"` and an optional `"secondaryFont"`.
- **WASM Schema Introspection**: Refactored `gen_sim_registry.py` and `DesignerRegistry` to embed C++ types during compilation, ensuring the simulator UI models unmapped components with fully populated default JSON properties on-click.
- **Blink State Consolidation**: Disabled the continuous colon 500ms flash toggle on the seconds separator, bringing it in line with official National Rail specifications where only the HH::MM colon flashes.

**Archive**: [.agents/plans/done/2c2d6708-ef88-405e-bc90-0281c833e573/](.agents/plans/done/2c2d6708-ef88-405e-bc90-0281c833e573/)

### Git Commit
Generated commit: 388b36f

## 2026-03-28 - Optimizing ESP32 Memory Footprint (Ad-hoc Session)

### Session Summary
Diagnosed the ESP32 RAM usage and successfully refactored `portalBuilder.py` to ensure Web Portal assets (`index.html`, `rss.json`) are decoupled into a dedicated `portalAssets.cpp` compilation unit rather than being injected directly into the `webHandlerManager` header file. Confirmed that the `72KB` active RAM footprint is standard operating behavior and verified that the Flash asset linking is functioning natively.

### Key Decisions
- **Compilation Unit Isolation**: Updated `portalBuilder.py` to produce a separate `.cpp` file for asset definitions to prevent duplicate symbol instantiations in C++.
- **Flash Validation**: Correctly analyzed the memory map (esp-idf `0x3F41...` ranges) via `xtensa-esp32-elf-nm`, confirming that assets are safely residing in Flash (`.rodata`/DROM) and are not consuming SRAM, despite `nm` outputting a `D` (data) segment tag.

### Git Commit
Generated commit: 7b8b123

## 2026-03-28 - TfL & Bus Board Memory Safety & Layout Upgrade (Session 2e1382f8)

### Session Summary
Successfully resolved a critical memory corruption issue in the TfL and Bus board firmware by refactoring data structures to a zero-copy architecture. Upgraded both boards to a high-fidelity 4-column layout (Order, Line/Route, Destination, Time) to match London transport standards. Replaced dangerous stack-allocated string pointers with static, persistent pointers for service numbering.

### Key Decisions
- **Zero-Copy Numbering**: Implemented a `static const char*` array (`serviceNumbers`) in `TflDataSource` and `BusDataSource`. This provides persistent, zero-overhead pointers for service position numbers ("1" through "20"), eliminating dangling pointers in the `serviceListWidget`.
- **High-Fidelity Layouts**: Updated `layoutDefault.json` and `layoutDefault.cpp` for both boards to a 4-column structure. Refactored `updateData()` in both board controllers to pass the `orderNum` pointer to the `serviceListWidget` conditionally based on the `config.showServiceOrdinals` setting.
- **Architectural Cleanup**: Fixed a syntax error in `modules/schedulerManager/schedulerManager.cpp` (missing `if` condition for change detection) that was blocking compilation and corrected loop scope issues in `TfLBoard::updateData()`.
- **House Style Alignment**: Audited and updated all modified modules to strictly adhere to v3.0 project standards, including Doxygen headers and step-by-step functional comments.

**Archive**: [.agents/plans/done/2e1382f8-65d9-410b-bfaa-53dbcf9bc7bf/](.agents/plans/done/2e1382f8-65d9-410b-bfaa-53dbcf9bc7bf/)

### Git Commit
Generated commit: 10e5e55

## Plan 00a761cb-6c18-468c-ba43-4bea43e5276d
**Date**: 2026-04-02
**Session**: 00a761cb-6c18-468c-ba43-4bea43e5276d
**Action**: Migration of Preprocessor Macros to Type-Safe Constants
**Summary**: Refactored the firmware's status reporting and scheduling systems to use type-safe `UpdateStatus` and `PriorityTier` enums. Resolved critical macro collisions between `PriorityTier` members (`LOW`, `HIGH`) and Arduino's hardware abstraction by adopting the `PRIO_` prefix. Standardized the `iDataSource` interface to return `UpdateStatus` and updated all data sources (National Rail, TfL, Bus, RSS, Weather) to comply. Validated the changes through a clean PlatformIO build and host-based unit tests.
**Commit**: 392af1a

## Plan 17c516ce-f569-43ec-997b-0a4881a74637
**Date**: 2026-04-02
**Session**: f0996eb9-83f8-4f69-a21f-1e7b8ad0cad6
**Action**: Refactoring dataManager Logging
**Summary**: Successfully replaced the legacy runtime `enableDebug` flags in `dataManager` with a global, compile-time `LOG_VERBOSE` (Level 5) logging tier. This architectural shift significantly reduces memory overhead and improves code maintainability by ensuring high-frequency "spam" logs are completely excluded from the binary unless explicitly enabled via `CORE_DEBUG_LEVEL`. Updated `Logger` library with the new level and unified the `dataManager` and `appContext` initialization signatures.
**Archive**: [.agents/plans/done/17c516ce-f569-43ec-997b-0a4881a74637/](.agents/plans/done/17c516ce-f569-43ec-997b-0a4881a74637/)
**Commit**: [Captured in session git history]

## Plan 44f14aa6-4a8e-4c1d-b18d-ae7bf9ebcb11
**Date**: 2026-04-02
**Session**: 54ace06a-60ad-4948-93f5-b71e7a39cf9b
**Action**: Relocating Attribution Constants
**Summary**: Refactored `nrAttribution`, `tflAttribution`, and `btAttribution` out of the global scope in `src/departuresBoard.cpp` into their respective board implementations (`nationalRailBoard.cpp`, `tflBoard.cpp`, `busBoard.cpp`). This removes minor architectural debt and strictly encapsulates display-only text within the correct domain controllers.
**Archive**: [.agents/plans/done/44f14aa6-4a8e-4c1d-b18d-ae7bf9ebcb11/](.agents/plans/done/44f14aa6-4a8e-4c1d-b18d-ae7bf9ebcb11/)
**Commit**: a89c7e3

## Plan 8a9a238e-dc7d-4e80-a935-5d897c2cd59f
**Date**: 2026-04-02
**Session**: 303ac83a-6266-463c-ad89-a347c37505ea
**Action**: Include Graph Optimization Execution
**Summary**: Optimized C++ include graph by removing heavyweight ESP32 libraries (`WiFi.h`, `LittleFS.h`) from global `.hpp` interfaces. Fixed ensuing compilation errors by restoring transitive dependencies for `Arduino.h` core types in (`systemManager.hpp`, `busDataSource.hpp`, `progressBarWidget.hpp`, `weatherClient.hpp`). Validated via `pio run -e esp32dev`.
**Archive**: [.agents/plans/done/8a9a238e-dc7d-4e80-a935-5d897c2cd59f/](.agents/plans/done/8a9a238e-dc7d-4e80-a935-5d897c2cd59f/)
**Commit**: 9638158

## Plan e62ac21e-0038-4996-a217-16b15ac43d02
**Date**: 2026-04-02
**Session**: 210a86ed-d0ef-40c2-a904-10ec2fe94f87
**Action**: RAII and Memory Management Refactor
**Summary**: Systematically migrated the core firmware from manual heap management (`new`/`delete`) to C++14 RAII paradigms using `std::unique_ptr` and `std::make_unique`. Refactored `systemManager`, `WebServerManager`, `WifiManager`, and all transport data sources. Resolved critical "incomplete type" build errors by moving constructors, destructors, and setters to `.cpp` files. Enforced project-wide documentation standards for `std::move()` ownership transfers. Verified via `pio run -e esp32dev`.
**Archive**: [.agents/plans/done/e62ac21e-0038-4996-a217-16b15ac43d02/](.agents/plans/done/e62ac21e-0038-4996-a217-16b15ac43d02/)
**Commit**: [Captured in session git history]

## Plan 8cea9a9b-2af4-45ea-a652-c71e3367672b
**Date**: 2026-04-02
**Session**: ebd99ff6-6d31-40db-a7c1-628c1035cbc2
**Action**: DI Refactor & Encapsulation
**Summary**: Eliminated floating global singletons (`displayManager`, `currentWeather`, `server`, `manager`) in favor of context-based service access via `appContext`. Removed deprecated, UI-bleeding callbacks from `systemManager`.
**Archive**: [.agents/plans/done/8cea9a9b-2af4-45ea-a652-c71e3367672b/](.agents/plans/done/8cea9a9b-2af4-45ea-a652-c71e3367672b/)
**Commit**: 2072c04
