# Implementation Plan - Fix NTP Synchronization and Strict DI for TimeManager

Audit findings revealed that `TimeManager::initialize()`, which handles NTP server configuration and initial time synchronization, is defined but never called. Additionally, the system currently relies on global `timeinfo` structures. Per user feedback, we will refactor the `clockWidget` to use strict Dependency Injection (DI) for time access.

## Proposed Changes

### [TimeManager & AppContext]

#### [MODIFY] [timeManager.hpp](lib/timeManager/timeManager.hpp)
- Remove `extern struct tm timeinfo` global declaration.
- Add a private `struct tm currentTime` member to `TimeManager`.
- Add `bool updateCurrentTime()` which calls `getLocalTime(&currentTime)`.
- Add `const struct tm& getCurrentTime() const` accessor.

#### [MODIFY] [timeManager.cpp](lib/timeManager/timeManager.cpp)
- Remove global `struct tm timeinfo`.
- Implement new accessors.

#### [MODIFY] [appContext.hpp](modules/appContext/appContext.hpp)
- Add `TimeManager& getTimeManager()` accessor.

#### [MODIFY] [appContext.cpp](modules/appContext/appContext.cpp)
- Call `timeManager.initialize()` during the `BOOTING` sequence in `tick()` after WiFi is ready.

### [Clock Widget Refactoring]

#### [MODIFY] [clockWidget.hpp](modules/displayManager/widgets/clockWidget.hpp)
- Add `TimeManager* timeMgr` private member.
- Update constructor to require a `TimeManager*`.

#### [MODIFY] [clockWidget.cpp](modules/displayManager/widgets/clockWidget.cpp)
- Remove `extern struct tm timeinfo`.
- Update `renderAnimationUpdate` and `render` to use `timeMgr->updateCurrentTime()` and `timeMgr->getCurrentTime()`.

#### [MODIFY] [headerWidget.cpp](modules/displayManager/widgets/headerWidget.cpp)
- Update to use `appContext->getTimeManager()` if available, otherwise fallback to local `getLocalTime` (or better, inject `TimeManager`).

### [Board Integration]

#### [MODIFY] [sleepingBoard.cpp](modules/displayManager/boards/systemBoard/sleepingBoard.cpp) (and others using `clockWidget`)
- Pass `context->getTimeManager()` when instantiating or initializing `clockWidget`.

## Verification Plan

### Automated Tests
- `pio run` to verify compilation and dependency linking.

### Manual Verification
1. **Boot Sequence**: Verify "Setting the system clock..." appears and succeeds.
2. **Strict DI**: Verify no `extern` globals are used for time in widgets.
3. **Clock Accuracy**: Confirm `clockWidget` updates correctly from the injected `TimeManager`.
