---
name: "Web Portal Refactor Phase 2: API Integration & CRUD Logic"
description: "Wired the newly developed frontend prototype directly to the C++ backend. Transitioned from legacy granular POST endpoints to a modern \"Unified Save\" JSON API that bridges System Settings, API Keys, a..."
created: "2026-03-14"
status: "DONE"
commits: ['642c158']
---

# Summary
Wired the newly developed frontend prototype directly to the C++ backend. Transitioned from legacy granular POST endpoints to a modern "Unified Save" JSON API that bridges System Settings, API Keys, and Boards into a single atomistic update.

## Key Decisions
- **Unified Save (`/api/saveall`)**: Frontend now aggregates all application state into one JSON blob. This minimizes HTTP overhead and guarantees transactional-like consistency when applying config changes.
- **Dynamic SPA Rendering**: Replaced static HTML scaffolding in `index.html` with vanilla JS Template literals that dynamically fetch and render arrays (e.g., Active Boards CRUD interface, API Key list).
- **In-situ Diagnostics Polling**: The frontend now polls `/api/status` every 10 seconds, binding live RSSI, Heap size, Uptime, and IP address to the expandable drawer UI without hard-refreshing the browser.

## Technical Context
