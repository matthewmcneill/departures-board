# Unified Testing Model & UI Unification

## Goal Description
The ESP32 is currently being overwhelmed by concurrent `/api/.../test` HTTP requests when navigating between tabs. The single-threaded WebServer and limited LwIP connection slots cannot handle simultaneous blocking requests to external APIs (e.g. 6 board tests + weather + feeds). This causes dropped connections ("HTTP ERROR"), broken pipes, and UI failures ("grey/red dots"). 

Furthermore, the "Active Displays" slots lack clear textual feedback, relying only on a colored dot, whereas the "API Keys" slots have mature, readable status badges containing text (e.g., "Active", "Testing...").

We will implement a unified "Test Task Queue" in JavaScript to serialize all diagnostic checks and visually unify the Display slot UI with the API Keys UI.

## Resource Impact Assessment (Embedded Systems)
- **Memory (RAM/Heap)**: 🟢 **Massive Improvement**. By serializing HTTP `fetch()` requests on the frontend, the ESP32 will only ever instantiate **one** `HTTPClient` instance at a time to test external connections. This resolves peak heap exhaustion.
- **Power**: 🟢 Prevents overlapping WiFi TX/RX spikes caused by concurrent TLS connections.
- **Security**: 🟢 No changes. Secrets remain isolated.

## User Review Required
None required. The queue delay has been set to 500ms as requested, and queue-clearing logic has been included.

## Proposed Changes

### Portal UI (Frontend Queue)

#### [MODIFY] [index.html](file:///Users/mcneillm/Documents/Projects/departures-board/portal/index.html)
- **Implement `app.enqueueTest(taskFn)` & `app.clearTestQueue()`:** 
  - Add `testQueue[]` and `isQueueRunning` to `app.state`.
  - Process queue iteratively using `async/await`.
  - Introduce a **500ms delay** `await new Promise(r => setTimeout(r, 500))` between queue tasks to allow ESP32 OS tasks (WiFi/LwIP/Watchdog) to breathe.
  - Implement `app.clearTestQueue()` to empty `testQueue[]`, and hook this into `app.switchTab` so changing tabs immediately aborts pending tests from the previous tab.
- **Refactor Test Routines:**
  - `testBoardAsync()`, `testWeather()`, `testFeed()`, and `testKeyAsync()` will be updated to wrap their internal `fetch` logic inside a closure pushed to `app.enqueueTest()`. 
  - UI updates indicating "Testing..." will execute *immediately* (outside the queue), while the actual HTTP `fetch` executes when its turn arrives in the queue.
  - Remove custom `runSequentialTests` timing loop for API keys. Instead, the keys tab will iterate through **all** configured keys (up to `MAX_KEYS`) and push each of them into the new global test queue.

### UI Unification (Styles)

#### [MODIFY] [index.html](file:///Users/mcneillm/Documents/Projects/departures-board/portal/index.html)
- **Display Status Unification:**
  - Update `renderBoards()` output for `.board-slot-status` to include a text span `<span class="board-status-text" id="board-status-text-${i}">Unknown</span>` directly next to the existing dot.
  - Update `testBoardAsync()` to inject readable text ("Active", "Offline", "Error") alongside the green/red/yellow dot manipulation, matching the `.key-status-text` implementation.
  - Add minimal flexbox CSS for `.board-slot-status` to vertically and horizontally center the text and dot side-by-side.

## Verification Plan

### Manual Verification
1. Load the Portal locally or on the device.
2. Rapidly switch between the Feeds, Settings, Keys, and Displays tabs.
3. Observe that tests enter a "Testing..." / "Checking..." state immediately.
4. Open the Network inspector and verify that `/api/boards/test`, `/api/weather/test`, etc. are strictly executing **one after the other** with a **500ms pause**, with no concurrent overlapping requests.
5. Verify textual status updates appear correctly on the Display boards.
