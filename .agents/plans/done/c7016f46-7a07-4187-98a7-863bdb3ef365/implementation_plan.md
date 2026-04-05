# Implementation Plan: 15-Second Post-Departure Refresh (RDM)

## Audit Checklist
- [x] House Style Guidelines
- [x] Architectural Standards
- [x] UI Design (N/A)
- [x] Resource Impact Assessment

## Goal Description
Introduce "intelligent polling" into the new Rail Data Marketplace (RDM) data provider (`nrRDMDataProvider.cpp`). By parsing the scheduled or estimated time of departure (`std`/`etd`) for the top train on the board, we can precisely calculate the delta between the current system time and the train's departure. We then schedule the next refresh cycle for exactly 15 seconds after that departure time, enforcing sensible minimums and maximums to protect API limits.

## Resource Impact Assessment

> [!NOTE]
> **Embedded Systems Profile**
> - **Flash/ROM**: Negligible increase (< 1KB) for additional time parsing logic and interval math.
> - **RAM (Static/Heap)**: No new heap allocations. Stack usage increases slightly during the parse phase due to temporary local variables.
> - **Power**: Dynamic polling ensures we only wake the Wi-Fi modem and execute HTTPS requests when necessary. Clamping the maximum polling to 45s prevents excessive sleep but maintains a reasonable power profile compared to a strict 30s poll.
> - **Security**: No impact on HTTPS/TLS layers.

## User Review Required

> [!IMPORTANT]
> **Polling Boundaries:** The plan implements a dynamic interval that targets `departure + 15s`. If a train is 2 minutes away, calculating `departure + 15s` would mean waiting ~135 seconds for the next refresh. To prevent the board from looking "frozen" (e.g. not receiving cancellation updates), we enforce a **maximum** wait time. I am proposing clamping the maximum wait time to **45 seconds** (matching the legacy DARWIN provider), and the minimum to **15 seconds** (e.g., if we fall behind).
> 
> Does standardizing on `Max = 45s, Min = 15s` sound correct to you, or would you prefer different outer bounds for the dynamic polling?

## Proposed Changes

### displayManager / boards / nationalRailBoard

#### [MODIFY] [nrRDMDataProvider.cpp](modules/displayManager/boards/nationalRailBoard/nrRDMDataProvider.cpp)
1.  **Include AppContext**: Import `appContext.hpp` and declare `extern class appContext appContext;` so we can access `getTimeManager()`.
    * *Architectural Note:* Using `extern` for context mapping violates strict Dependency Inversion (DIP), but is used here for parity with the existing `nrDARWINDataProvider` as it minimizes blast radius. Future sweeping architecture refactors should replace `extern` with constructor injection.
2.  **Calculate Wall Clock Delta**: Inside `executeFetch()`, if `stationData->numServices > 0`:
    *   Call `appContext.getTimeManager().updateCurrentTime()` to grab the latest RTC time.
    *   Inspect `stationData->service[0]`.
    *   If `etd` is valid (e.g., "10:45"), use that. Otherwise, use `sTime` (`std`).
    *   Convert `HH:MM` into seconds-of-the-day and compare it against the current time (handling midnight rollovers).
3.  **Compute Custom Interval**:
    *   Calculate `delay_until_departure = train_time_seconds - current_time_seconds`.
    *   Set `target_interval = (delay_until_departure + 15) * 1000`.
    *   Clamp `target_interval` between minimum (15,000ms) and maximum (45,000ms).
4.  Apply this calculated interval using `setNextFetchTime(millis() + target_interval)`.

## Open Questions

None beyond the polling boundary bounds.

## Verification Plan

### Automated Tests
-   Verify code compilation natively to ensure the time manager API is accessed correctly.

### Manual Verification
-   Start the departures board and connect to the Serial monitor.
-   Wait for a train departure to approach.
-   Observe the `DATA_RDM` logs to visually verify standard 45s polling, followed by dynamic shrinkage as the train departure closes in (e.g. `Intelligent Polling: Target=10:35, Delta=12s, Delay=27s`), concluding with a refresh exactly 15 seconds after the displayed departure time.
