# Plan: Fix Device Hangs and Network Robustness

## Problem
The device firmware "hangs" (animations and clock stop) when fetching data from APIs, especially if the connection is slow or the request fails. This is caused by blocking loops in `WiFiClientSecure` operations that do not yield to the display manager. Additionally, passing empty IDs to APIs results in unnecessary 404 errors (Code 4).

## Proposed Changes

### [Component] Data Sources

#### [MODIFY] [busDataSource.cpp](modules/displayManager/boards/busBoard/busDataSource.cpp)
- Add guard to `updateData()`: Return `UPD_NOT_CONFIGURED` if `busAtco` is empty.
- Add `callback()` (which yields to display) in the `available()` wait loops.
- Improve `LOG_WARN` for `UPD_HTTP_ERROR` to include the status line.

#### [MODIFY] [nationalRailDataSource.cpp](modules/displayManager/boards/nationalRailBoard/nationalRailDataSource.cpp)
- Add guard to `updateData()`: Return `UPD_NOT_CONFIGURED` if `crsCode` or `nrToken` is empty.
- Add `callback()` in the `available()` wait loops.

#### [MODIFY] [tflDataSource.cpp](modules/displayManager/boards/tflBoard/tflDataSource.cpp)
- Add guard to `updateData()`: Return `UPD_NOT_CONFIGURED` if `tubeId` is empty.
- Add `callback()` in the `available()` wait loops.

### [Component] System Manager

#### [MODIFY] [systemManager.cpp](modules/systemManager/systemManager.cpp)
- Add handling for `UPD_NOT_CONFIGURED` (assume code 9) to prevent aggressive retries when a board isn't set up yet.

## Verification Plan

### Automated Tests
- Run `tests/help_text.spec.ts` to ensure no regression in portal state.
- Manual verification of serial logs after flashing to ensure "hangs" are replaced by smooth animations during fetches.

### Manual Verification
1. Flash `esp32dev` environment.
2. Monitor serial output.
3. Observe animations (scrollers/clocks) while "Triggering active board data update" is logged.
4. Verify that empty boards no longer trigger Code 4 logs.
