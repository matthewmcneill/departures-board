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
