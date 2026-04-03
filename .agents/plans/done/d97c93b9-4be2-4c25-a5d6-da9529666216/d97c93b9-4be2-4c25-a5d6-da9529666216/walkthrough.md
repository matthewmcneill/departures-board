# National Rail Data "No-Service" Fix

I investigated the issue where the National Rail Boards were failing to display service data despite receiving a valid XML payload from Darwin. The log outputs indicated that `numServices` was returning zero at the end of the data fetch cycle. 

## 1. The `tagPath` Suffix Failure
When `xmlStreamingParser` evaluates deep nested XML with multiple consecutive closed tags (e.g., closing out the last service block), the historical tags reset their identifiers to `"??`. This caused the dynamically built `tagPath` to evaluate to `??/lt8:service/lt4:std` rather than tracking all the way back to `lt8:trainServices`. 

As a result, the check `tagPath.endsWith("lt8:service/lt4:std")` failed to match. 
**Fix:** All `endsWith()` strict validations inside the `value(...)` XML listener method have been replaced with the more resilient `indexOf(...) != -1` to ensure we properly trigger the event handler loop regardless of depth cache clearing. 

## 2. Integer Promotion Underflow (`size_t` bounds checking)
The second failure prevented the system from saving the data even when the tag was triggered. During recent codebase refactoring, `NR_MAX_SERVICES` and related structure bounds were shifted to `constexpr size_t`.

When a new XML response begins iterating, the internal service `id` is reset to `-1`. 
The condition: `if (id < NR_MAX_SERVICES - 1)` essentially evaluated `int -1 < size_t 9 - 1`. 
Due to strict C++ type promotion rules, the signed `int` was implicitly cast to an unsigned integer space, causing `id` to underflow its boundaries to `UINT_MAX (4,294,967,295)`. As such, `4,294,967,295 < 8` evaluated to `false`, silently skipping all array populations.

**Fix:** Changed `constexpr size_t` back to `constexpr int` for `NR_MAX_SERVICES`, `NR_MAX_CALLING`, `NR_MAX_LOCATION`, and `NR_MAX_MSG_LEN` in `nationalRailDataSource.hpp`.

## 3. Pagination State Wipeout Bug
The user identified an issue where National Rail boards would not scroll despite having multiple (e.g. 9 for Epsom) services visible.
Upon investigation, the `layoutNrDefault` configures `servicesWidget` with a `scrollDwellMs` of `5000` (meaning it waits 5 seconds to scroll). However, `NationalRailBoard::tick()` executes a routine every `4000` ms to toggle the `viaToggle` boolean and immediately calls `populateServices()`.
The original implementation of `populateServices()` aggressively called `clearRows()` on **both** the Header row (`row0Widget`) and the scrolling data (`servicesWidget`). This had the unintended consequence of completely zeroing out the `serviceListWidget` animation state machine every 4 seconds, resetting its `dwell` timer just before it had a chance to reach 5 seconds and begin pagination.

**Fix:** Refactored `NationalRailBoard::populateServices(bool row0Only)` to accept an override flag. During the 4000ms `viaToggle` flips, we now target `true`, allowing `servicesWidget` to retain its data pointers and internal pagination variables uninterrupted.

## Verification
I ran a hardware verification debug sequence over the ESP32 terminal to confirm the populated array sizes incremented cleanly, observing:
> `Service 0: 17:52 to London Waterloo (Exp: On time, Plat: 1)`

The codebase has been refactored, the debug tracers have been cleanly reverted, and system memory remains stable.
