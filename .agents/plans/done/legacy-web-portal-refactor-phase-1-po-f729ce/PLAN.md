---
name: "Web Portal Refactor Phase 1: Portal Infra & Backend Refactor"
description: "Established the parallel infrastructure for the modern web portal and refactored the backend web server logic. This phase enables the development of a modern SPA without disrupting the legacy web inte..."
created: "2026-03-14"
status: "DONE"
commits: []
---

# Summary
Established the parallel infrastructure for the modern web portal and refactored the backend web server logic. This phase enables the development of a modern SPA without disrupting the legacy web interface.

## Key Decisions
- **Parallel Evolution Strategy**: Created a dedicated `/portal` directory for modern SPA assets, allowing side-by-side operation with the legacy `/` interface.
- **Automated Asset Pipeline**: Implemented `portalBuilder.py` to gzip and hexify assets into a C++ header (`portalAssets.h`). This ensures that the frontend remains zero-dependency (no external CDNs required).
- **Decoupled Handler Manager**: Extracted portal-specific routing into a standalone `WebHandlerManager` class, using dependency injection for `WebServer` and `ConfigManager`. This improves codebase modularity and testability.
- **uiproto Standard**: Adopted the bottom navigation and sticky action patterns from the prototype, ensuring a premium "app-like" experience on embedded hardware.

## Technical Context
