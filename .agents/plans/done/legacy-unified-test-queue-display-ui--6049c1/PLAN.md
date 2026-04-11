---
name: "Unified Test Queue & Display UI Unification"
description: "Implemented a centralized `app.state.testQueue` in the portal to serialize all diagnostic requests (Boards, Keys, Feeds, Weather). This prevents the ESP32 from being overwhelmed by concurrent SSL/API ..."
created: "2026-03-17"
status: "DONE"
commits: ['175be62']
---

# Summary
Implemented a centralized `app.state.testQueue` in the portal to serialize all diagnostic requests (Boards, Keys, Feeds, Weather). This prevents the ESP32 from being overwhelmed by concurrent SSL/API requests. Unified the diagnostic UI for the "Displays" tab to match the "API Keys" style, adding explicit status text labels.

## Key Decisions
- **Serialized Diagnostic Queue**: All diagnostic tests now run sequentially with a 500ms breather, resolving the "HTTP ERROR" and "Red Dot" failures caused by socket exhaustion.
- **Tab Context Sensitivity**: Integrated `clearTestQueue` into the tab switcher to immediately cancel stale tests from previous views.
- **Visual Feedback Unification**: Added status text labels ("Testing...", "Active", "Offline") to board slots, improving UX and consistency across system diagnostic views.
- **Race Condition Resolution**: Moved health check triggers to execute after the DOM is fully rendered in `renderBoards` to ensure target element availability.

## Technical Context
