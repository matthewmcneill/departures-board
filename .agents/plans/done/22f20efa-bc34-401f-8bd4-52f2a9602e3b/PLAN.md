---
id: 22f20efa-bc34-401f-8bd4-52f2a9602e3b
title: Stabilizing Web Portal (Mobile Crash Fix)
status: DONE
author: antigravity
created: 2026-03-28T09:41:00Z
updated: 2026-03-28T10:39:30Z
---

# Stabilizing Web Portal (Mobile Crash Fix)

The user report of system instability when loading the portal on mobile was traced to concurrent high-memory web requests (index.html is 152KB, config.json is 4KB) exhausting the ESP32's 320KB RAM and particularly the available 34KB max heap block.

## Changes
- **WebHandlerManager Concurrency Guard**: Implemented an atomic semaphore (`activeHighMemRequests`) to serialize the serving of large assets.
- **SVG Symbol Library**: Consolidated ~20 inline SVG icons into a centralized `<symbol>` library in `index.html`. This reduced the DOM size and stabilized icon rendering across different board types.
- **Zero-Heap Logging**: Replaced `String` object usage in web server diagnostics with `snprintf` to avoid heap fragmentation during request handling.
- **UI Restoration**: Corrected sizing and alignment regressions for National Rail, TfL, and Key Registry icons while maintaining the original design's indicator rows and badges.

## Verification
- Verified via Playwright subagent: 100% test pass on web components.
- Verified on hardware (esp32dev): Successful boot and OOM-free portal access on a mobile phone with concurrent fetch activity.
- Serial logs confirm semaphore contention logic works (`WEB_API: Heavy request rejected (Heap/Concurrency Guard)`).

## Commit
`Refactor: Stabilize Web Portal for Mobile Load & SVG Optimization (v3.0)`
