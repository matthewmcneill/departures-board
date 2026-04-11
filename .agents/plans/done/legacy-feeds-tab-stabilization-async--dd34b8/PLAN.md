---
name: "Feeds Tab Stabilization & Async Diagnostics"
description: "Finalized the FEEDS tab UI and back-end diagnostic logic. Implemented asynchronous auto-testing for News and Weather feeds, stabilized \"Test Feed\" buttons at 44px height, and added a grey \"UNCONFIGURE..."
created: "2026-03-17"
status: "DONE"
commits: ['6896809']
---

# Summary
Finalized the FEEDS tab UI and back-end diagnostic logic. Implemented asynchronous auto-testing for News and Weather feeds, stabilized "Test Feed" buttons at 44px height, and added a grey "UNCONFIGURED" state for deselected weather keys. Migrated the RSS feed list to a separate `rss.json` file to simplify management and decoupling from the main HTML.

## Key Decisions
- **Button Stabilization**: Applied fixed `height: 44px` and zero vertical padding to `.btn-test-action` to eliminate layout shifts during the "Testing..." state.
- **RSS JSON Migration**: Moved the RSS feed list to a separate `rss.json` asset served via the new `handleRSSJson()` endpoint, allowing for easier feed updates without re-building the entire portal.
- **Asynchronous Auto-Testing**: Configured the FEEDS tab to trigger background connectivity tests immediately upon opening, allowing diagnostic dots to populate without stalling the UI.
- **Multi-Key Weather Logic**: Enhanced `weatherClient` to support specific key IDs and override tokens, enabling real-time validation of unsaved keys in the portal.

## Technical Context
