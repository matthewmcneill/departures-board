# Project Log

## 2026-03-15 - System Modules and WiFi Manager Integration

### Session Summary
Executed an architectural refactoring to strictly enforce dependency injection and eliminate global singletons. Moved `appContext` to its own namespace (`modules/appContext`) to resolve LDF scoping issues and migrated `WiFiConfig` to `modules/wifiManager`. `include/buildOptions.h` was consolidated into `src/departuresBoard.hpp` as the primary configuration header. 

### Key Decisions
- **`appContext` Module**: Isolated `appContext` to fix PlatformIO Library Dependency Finder (LDF) cyclic dependency parsing when generic libraries included `.hpp` files from the consumer `src/` directory.
- **Global `wifiManager` Removal**: Refactored `wifiManager` into a scoped member of `appContext`, transitioning all legacy handlers to retrieve it via `appContext.getWifiManager()`. 
- **House Style Compliance**: Rewrote all refactored file headers to strictly adhere to the `camelCase.hpp` and Doxygen standard. 

### Git Commit
Generated commit: 0c8551e


## 2026-03-15 - State Machine and Boot Initialization Refactor

### Session Summary
Refactored the initial boot sequence in `appContext` and `DisplayManager` to cleanly transition through states (`BOOTING`, `WIFI_SETUP`, `BOARD_SETUP`, `RUNNING`) without premature board activation or API requests under unconfigured conditions. Added a robust state management document (`AppContextStateMachine.md`).

### Key Decisions
- **`hasConfiguredBoards()` Implementation**: Modified the configuration check to evaluate `.complete` flags natively instead of blindly trusting `MAX_BOARDS` index parsing, resolving a bug where default incomplete boards bypassed the Web Setup phase.
- **Display Transition Integrity**: Fixed `appContext::tick()` to explicitly trigger `showBoard(SYS_HELP_CRS)` during the precise transition from `BOOTING` to `BOARD_SETUP`. 
- **Splash Screen Lockout**: Restructured `DisplayManager::applyConfig` to block configured boards from replacing the system `splashBoard` or `wifiWizard` until `appContext` fully transitions into the `RUNNING` state.

### Git Commit
Generated commit: 18f4bca

## 2026-03-14 - Additional Drawing Primitives

### Session Summary
Added a suite of common drawing primitives (`drawBox`, `drawLine`, `drawCircle`, `drawRoundedBox`, `drawTriangle`) to the `drawingPrimitives` helper library to simplify board-specific UI development.

### Key Decisions
- **API Harmonization**: Implemented a unified `isFilled` parameter across all applicable primitives, abstracting away the disjointed `u8g2` naming convention (`drawBox` vs `drawFrame`).
- **Triangle Extension**: Added custom line-drawing logic to `drawTriangle` to enable unfilled triangles, a feature missing from the base `u8g2` library.
- **Helper Utility Justification**: Validated the continued use of the `drawingPrimitives` library as a high-level UI abstraction layer for text centering, truncation, and screen-aware geometry.

### Git Commit
Generated commit: be78a5c

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


## 2026-03-16 - Portal UI Polish: Eye Icons, NR & TfL Logos

### Session Summary
Corrected the inverted logic for the password visibility toggle and upgraded the National Rail and TfL provider logos to their official SVG versions. Verified the fixes on live hardware across multiple deployment iterations. Addressed initial stale-code issues through clean builds and forced browser cache bypass.

### Key Decisions
- **CSS-Driven Toggle**: Transitioned the password visibility logic to rely on the CSS `active` class. This resolved conflicts with `!important` rules in the dark-mode CSS and ensured hardware-consistent performance.
- **Icon Logic Harmonization**: Standardized the visibility icon states to use the "Slashed Eye" for the visible password state and the "Open Eye" for the masked state, matching modern digital accessibility patterns.
- **Provider Logo Refresh**: Replaced outdated logos in the `PROVIDER_LOGOS` registry and selection modals with high-resolution, branded SVGs (National Rail double-arrow and TfL Roundel with `#0019A8` fill).
- **Automated Validation Integration**: Verified all UI changes using both local simulation and live hardware subagent testing, ensuring that user-provided formatting changes did not introduce regressions.

### Git Commit
## 2026-03-16 - Sequential API Key Validation & Memory Optimization

### Session Summary
Implemented sequential API key validation in the web portal to prevent ESP32 network module saturation and handled stack overflow issues by offloading SSL clients to the heap. Improved visual feedback for API tests and network disconnection states.

### Key Decisions
- **Sequential Validation Logic**: Orchestrated API key tests to execute one-by-one with a 1-second delay. This ensures hardware stability during the intensive SSL handshake process and provides a predictable user experience with sequential status dot updates.
- **Heap-Allocated SSL Clients**: Refactored `NationalRailDataSource` and `TfLDataSource` to use heap-allocated `WiFiClientSecure` objects. This prevents stack overflow crashes during complex TLS handshakes without introducing long-term fragmentation (short-lived allocations).
- **Network Error Observability**: Integrated a `isWifiPersistentError` method and updated `wifiStatusWidget` with a blinking logic (after 30s) to clearly signal prolonged connectivity loss on the hardware display.
- **Safe Password/Key Masking**: Implemented a "No Change" detection pattern for password and API key fields. The system now ignores placeholders (`••••••••`) during saves, preventing accidental erasure of existing secrets.

### Git Commit
## 2026-03-16 - Repository-wide House Style Update: Refactoring Attribution

### Session Summary
Applied the new refactoring attribution line to the standard module header across the entire C++ codebase. This involved updating 87 files across `src`, `modules`, and `lib`. The `house-style-documentation` skill was also updated to ensure all future headers include this line.

### Key Decisions
- **Automated Replacement**: Used `sed` with a repository-wide `find` search to ensure 100% consistency across all `.cpp` and `.hpp` files.
- **Documentation Alignment**: Synchronized the project's "House Style" definition in the agent skills to prevent future agents from accidentally reverting or omitting the new attribution.
- **Build Optimization**: Configured `default_envs` in `platformio.ini` to only build hardware targets by default. This resolves the false-positive failure in the `native` environment during a standard `pio run`, while maintaining functional unit tests via `pio test`.

### Git Commit
Generated commits: 10a9bbf, b185905
