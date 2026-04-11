---
name: "Purging Legacy Yield Callbacks"
description: "Successfully purged the obsolete yieldCallback mechanism from the firmware architecture. This removed redundant function pointers and registration logic from appContext, weatherClient, rssClient, and ..."
created: "2026-04-02"
status: "DONE"
commits: []
---

# Summary
Successfully purged the obsolete yieldCallback mechanism from the firmware architecture. This removed redundant function pointers and registration logic from appContext, weatherClient, rssClient, and transport data sources (TfL, National Rail). Updated board controllers to remove call-site overhead and modernized the AsyncDataRetrieval.md and WeatherSystemDesign.md documentation to reflect the current dual-core FreeRTOS task yielding model. Verified the build stability with pio run.

## Technical Context
- [sessions.md](sessions.md)
- [context_bridge.md](context_bridge.md)
- [task.md](task.md)
