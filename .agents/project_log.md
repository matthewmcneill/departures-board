# Project Log

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
Generated commit: [COMMIT_ID]

