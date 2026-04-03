# Goal description
Reduce the static memory footprint of the `NationalRailStation` data structure to prevent Out Of Memory (OOM) crashes when multiple National Rail boards are configured simultaneously.

Currently, the system crashes because adding a new National Rail board instantly permanently reserves ~20KB of heap. When the heap borders on exhaustion, `AsyncTCP` throws an unhandled exception when trying to handle a background heartbeat connection, resulting in a hardware abort.

## Analysis of the Crash Root Cause
Your observation perfectly explains the sequence of events:
1. Every `NationalRailBoard` you configure creates two `NationalRailStation` structs in the background (`stationData` and `renderData` inside the `nationalRailDataSource` plugin) to double-buffer incoming XML.
2. The `NationalRailStation` struct has an array of 9 services. Each service contains a 450-byte text buffer `char calling[450]` to hold the list of calling points, plus text buffers for the destination, operator, service message, etc.
3. This means a single `NationalRailStation` struct is roughly **10,000 bytes**. With double-buffering, each National Rail board you add to your config permanently locks away **~20 KB** of precious ESP32 heap!
4. When you added the new board, the heap was permanently pushed below the safety margin. When your browser's background auto-refresh requested `/api/status` a few "moments after", `AsyncTCP` attempted to allocate `120 bytes` for the socket client context, failed (because the heap was too small or completely fragmented by the ~20KB block), threw `std::bad_alloc`, and triggered `abort()`.

## User Review Required
We need to drastically shrink the footprint of `NationalRailStation` so you can safely run multiple boards. 

> [!CAUTION]
> Reducing the size of these buffers means we are imposing hard limits on data length. If a train has more calling points than the buffer can hold, it will get truncated. 

## Proposed Changes

### 1. Extract First-Service Details (The Optimization)
You're exactly right—we don't need to reduce the size of the buffers; we just need to stop allocating them nine times per board! 

If we look at `NationalRailBoard.cpp`, the board *only ever* scrolls the calling points, origin, last seen location, and service messages for the very first train (`data->service[0]`). The other 8 trains displayed in the list only need to show the destination, platform, and ETA.

We will restructure the structs as follows:

#### [MODIFY] `modules/displayManager/boards/nationalRailBoard/nationalRailDataSource.hpp`
Remove the heavy strings from `NationalRailService` and elevate them to `NationalRailStation` to act as properties of the "Next Train":

```cpp
struct NationalRailService {
    // We KEEP these for all 9 services (Total: ~169 bytes)
    char sTime[6];
    char destination[NR_MAX_LOCATION];
    char via[NR_MAX_LOCATION];
    char etd[11];
    char platform[4];
    bool isCancelled;
    bool isDelayed;
    int trainLength;
    byte classesAvailable;
    char opco[50];
    NrServiceType serviceType;
    
    // We REMOVE these from the array
    // char calling[450];
    // char origin[45];
    // char lastSeen[45];
    // char serviceMessage[400];
};

struct NationalRailStation {
    // Existing Station Metadata...
    char location[NR_MAX_LOCATION];
    // ...

    // ADD First Train Specific buffers here (Only exists ONCE)
    char firstServiceCalling[NR_MAX_CALLING];
    char firstServiceOrigin[NR_MAX_LOCATION];
    char firstServiceLastSeen[NR_MAX_LOCATION];
    char firstServiceMessage[NR_MAX_MSG_LEN];

    int numServices;
    NationalRailService service[NR_MAX_SERVICES];
    uint32_t contentHash;
};
```

**Why this works miracles:**
- **Current Size:** The `NationalRailService` struct is `~1,109 bytes`. An array of 9 is `~9,981 bytes`.
- **New Size:** A stripped `NationalRailService` is `~169 bytes`. An array of 9 is `~1,521 bytes`. Adding back the single `940-byte` block for the first train brings the total payload size to **~2,461 bytes**.
- Since each board double buffers, the RAM cost per board plummets from **~20.4 KB** down to **~5.1 KB**.

We save **~15 KB** per board, preserve the 9-train cache, *and* maintain the 450-character limit for massive routes!

#### [MODIFY] Parsing & Rendering (Plumbing)
- Update `nationalRailDataSource.cpp`'s `value()` parser to only copy `calling`, `origin`, and `lastSeen` if `id == 0`.
- Update `NationalRailBoard.cpp` to pull the scrolling message from `data->firstServiceCalling` instead of `data->service[0].calling`.

### 2. Defensive Memory Allocation (try/catch)
You raised an excellent point. While the specific crash we traced earlier happened inside the *internal* `AsyncTCP` library (which we can't easily patch without forking the library), we absolutely should protect our native allocations! If our code attempts to allocate a board or a data structure and fails, we should gracefully log an error and continue rather than bringing the hardware down.

#### [MODIFY] `modules/displayManager/boardFactory.cpp`
Currently, `BoardFactory` uses raw `new Board()`. We will wrap the core factory allocation block in a `try/catch(const std::bad_alloc& e)`:
```cpp
    try {
        switch (boardType) {
            case BoardType::NR_BOARD: return new NationalRailBoard(context);
            // ...
        }
    } catch (const std::bad_alloc& e) {
        LOG_ERROR("SYSTEM", "CRITICAL OOM: Failed to allocate Board on heap!");
        return nullptr; // Gracefully continue; DisplayManager already checks if(board)
    }
```

#### [MODIFY] `modules/webServer/webHandlerManager.cpp`
There are several places where we instantiate API Test sources to the heap asynchronously. We will wrap these in `try/catch` logic so the web dashboard throws a `500 Server Error` instead of abruptly crashing the board when testing Data Source keys.

## Open Questions
No further questions. This restructuring guarantees we keep the massive 450-character routes while saving 15 KB per board!

## Verification Plan
### Automated Tests
1. Flash changes via `/flash-test`.
2. Add 3 parallel National Rail boards via the Web UI to verify no OOM crashes occur during saving.
3. Monitor `Free Heap` telemetry in the UI.
