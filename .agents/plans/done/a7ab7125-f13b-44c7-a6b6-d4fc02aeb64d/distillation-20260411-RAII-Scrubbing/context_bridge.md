---
title: National Rail Darwin RAII Refactor
distilled_at: 2026-04-11T13:16:45Z
original_plan_id: standalone
artifacts:
  - context_artifacts/adr_raii_scrubbing.md
  - context_artifacts/graveyard_port_contention.md
---

# Executive Summary
In this session, we refactored the `nrDARWINDataProvider` module's memory scrubbing logic. We replaced the legacy `goto scrub_and_exit_fetch` pattern with a modern **RAII ScrubGuard** local struct. This resolved "jump to label" compilation errors and significantly improved code maintainability and localized variable scoping. The solution has been verified with successful automated builds and real-world hardware deployment on the Nano ESP32.

# Next Steps
1.  **Monitor Field Stability**: Ensure no heap fragmentation occurs over prolonged runtimes with the new RAII guard.
2.  **Module Propagation**: Evaluate applying the `ScrubGuard` pattern to other parsing-heavy modules (`rssClient`, `weatherClient`) to unify the cleanup architecture.
3.  **Heap Diagnostics**: Use the `diagnostics.py` skill periodically to track long-term memory high-water marks.

# Deep Context Menu

> [!WARNING]
> Do NOT read the detailed artifacts below unless your current task explicitly requires the deep context. If needed, use your read tool on `context_artifacts/[filename]`.

- `context_artifacts/adr_raii_scrubbing.md` - Technical rationale for the RAII transition and its benefits for the Hybrid Memory Model.
- `context_artifacts/graveyard_port_contention.md` - Analysis of the "Device not configured" flash failure and port contention on macOS.
