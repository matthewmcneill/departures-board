---
name: "Weather & Config v2.1 Refactor"
description: "Finalized the weather system refactor and configuration migration to Version 2.1. This included moving to an injectable, multi-location weather architecture and standardizing the configuration schema."
created: "2026-03-14"
status: "DONE"
commits: ['96ccdb6']
---

# Summary
Finalized the weather system refactor and configuration migration to Version 2.1. This included moving to an injectable, multi-location weather architecture and standardizing the configuration schema.

## Key Decisions
- **Injectable Weather**: Decided to have each `iDisplayBoard` own its own `WeatherStatus` object. This allows per-board weather tracking and refresh cycles.
- **Config v2.1**: Renamed `mode` to `type` and `boardMode` to `boardType` for better semantic clarity.
- **Automatic Migration**: Implemented logic in `ConfigManager` to detect v1.0/v2.0 configs and automatically upgrade/save them as v2.1 on first load.
- **Stateless Weather Client**: Refactored `weatherClient` to remove all internal state and simply update a provided `WeatherStatus` reference.

## Technical Context
