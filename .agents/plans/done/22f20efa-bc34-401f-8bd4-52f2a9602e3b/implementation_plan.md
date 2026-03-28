# Debugging System Stability (Mobile Portal Load)

The user reports that the system becomes unstable or crashes when loading the web portal from a mobile device. This is due to memory pressure and high-concurrency TCP connections exceeding the 34KB max heap block.

## User Review Required

> [!IMPORTANT]
> **Web Request Serialization**: To prevent concurrent memory spikes, I will implement a **Semaphore/Mutex** in the web server. This ensures that only one high-memory request (like serving the 152KB HTML) is processed at a time, even if the phone opens multiple connections.

## Proposed Changes

### Phase 1: Instrumentation & Identification (Diagnostic)

#### [MODIFY] `modules/webServer/webHandlerManager.cpp`
- **Memory-Safe Logging**: All diagnostics will use the `Logger` library. I will use a local `char` buffer and `snprintf` to format stats before passing them to `LOG_INFO` or `LOG_DEBUG`, avoiding any `String` object allocations or direct `Serial` calls.
- **Diagnostics**: Log `ESP.getFreeHeap()` and `ESP.getMaxAllocHeap()` at the start of every web handler using the above pattern.

### Phase 2: Payload Optimization (The "Diet")

#### [MODIFY] `web/index.html`
- **SVG Symbol Library**: Move all 18+ icon instances (Rail, Tube, Bus, Weather, Clock, Eyes) into a hidden `<symbol>` library at the top of the body.
- **Unification**: Replace emoji icons in the "Add Display" and "API Key" modals with these high-fidelity SVG symbols for a professional, unified look.
- **JS Cleanup**: Update the `app` object to pull SVG content via `<use href="#id">` instead of storing raw XML strings in variables.

### Phase 3: Backend Mitigation (Serialization)

#### [MODIFY] `modules/webServer/webHandlerManager.cpp`
- **Request Semaphore**: Wrap `handlePortalRoot` and `/api/config` in a local semaphore/atomic counter. 
- **Documentation**: I will add comprehensive internal documentation explaining that this serialization is required to protect the **34KB Max Block Size** during asynchronous mobile browser connections, where multiple parallel requests for the 152KB HTML would otherwise trigger a heap crash.

## Resource Impact Assessment

| Resource | Impact | Mitigation |
|----------|--------|------------|
| **RAM (Heap)** | **Critical** | Deduplication removes ~3KB of parsing overhead. Semaphore prevents OOM crashes. |
| **Flash** | **Low** | Minor increase due to the semaphore logic and diagnostic strings. |
| **CPU** | **Low** | No measurable impact; the server is already asynchronous. |
| **Security** | **None** | No changes to authentication or data privacy. |

## Verification Plan

### Automated Tests
- Playwright tests with `mobile` emulation, checking for `200 OK` and a significant reduction in `index.html` byte count.
- Stress test: Rapidly hitting `/web` and `/api/config` concurrently to verify the semaphore logic.

### Manual Verification
- Measuring "Free Heap" and "Max Block Size" during portal load on a real mobile device via serial logs.
