# Implementation Plan - Hardware Status Restoration & fragmentation Monitoring

The user reported that hardware status bars were missing and requested that the firmware build date be dynamically sourced from `src/buildTime.hpp`. Diagnostic bars should be color-coded (Red/Amber/Green) consistent with the existing health monitoring.

## Proposed Changes

### Backend (C++)

#### [MODIFY] [webHandlerManager.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/webServer/webHandlerManager.cpp)
- Include `../../src/buildTime.hpp` to access build metadata.
- In `handleGetStatus`:
    - Add `build` string using `BUILD_DATE_PRETTY`.
    - Add `min_heap` using `ESP.getMinFreeHeap()`.
    - Add `max_alloc` using `ESP.getMaxAllocHeap()`.

### Frontend (HTML/JS)

#### [MODIFY] [index.html](file:///Users/mcneillm/Documents/Projects/departures-board/web/index.html)
- Update `app.updateStatusUI()`:
    - Update the `#fw-build-text` element with the `build` date from the API.
- Update `app.renderHardwareStatus()`:
    - **Add diagnostic bars**:
        1. **Minimum Free Heap** (Low-Water Mark).
        2. **Largest Allocatable Block** (Fragmentation monitoring).
    - **Consistent Color Coding (RAG)**:
        - **Memory (Free / Min Free)**:
            - **Green**: > 80 KB
            - **Amber**: 50 KB - 80 KB
            - **Red**: < 50 KB (Consistent with the system health dot)
        - **Largest Free Block**:
            - **Green**: > 64 KB (Required for screen buffers/screenshots)
            - **Amber**: 32 KB - 64 KB (Tight for large JSON feeds)
            - **Red**: < 32 KB (Critical fragmentation; allocations will likely fail)
        - **Temperature**:
            - **Green**: < 50°C
            - **Amber**: 50-70°C
            - **Red**: > 70°C
        - **Storage (Used)**:
            - **Green**: < 70% Usage
            - **Amber**: 70-90% Usage
            - **Red**: > 90% Usage

### Test Rig (JS)

#### [MODIFY] [server.js](file:///Users/mcneillm/Documents/Projects/departures-board/test/web/server.js)
- Update the mock `/api/status` endpoint to include `build`, `min_heap`, `max_alloc`, `temp`, and `storage_total`.

## Verification Plan

### Automated Tests
- `pio run` to verify C++ compilation.
- Use `browser_subagent` to verify all 5 bars are visible, correctly labeled, and colored according to the new thresholds.

### Manual Verification
- Confirm the "Build" date in the UI matches the value in `src/buildTime.hpp`.
