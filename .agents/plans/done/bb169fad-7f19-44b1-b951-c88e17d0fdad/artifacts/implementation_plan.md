# [Boot Sequence Optimization & Fetch Guarding]

Optimize the boot sequence to reduce the time-to-running state and prevent redundant/failing network fetches during initialization on the ESP32.

## Requirement Review Status
[x] Reviewed by house-style-docs - passed
[x] Reviewed by oo-expert - passed
[x] Reviewed by embedded-systems - passed

## Goal Description
The current boot sequence is slowed down by a "network storm" where all departure boards attempt their initial data fetch simultaneously before the system has finished its core setup (NTP sync, mDNS, etc.). This is further exacerbated by unconfigured RSS/Weather clients attempting fetches that fail with timeouts or auth errors. This plan defers initial orchestration and adds silent guards to problematic clients.

## User Review Required
> [!IMPORTANT]
> This change will defer the initial data fetch for departure boards until *after* the "RUNNING" state is reached. This means the loading screen will transition to the board UI slightly faster, but the boards will initially show "Loading..." for a few seconds while they perform their first fetch in the background.

## Proposed Changes

### [Component: appContext]
#### [MODIFY] [appContext.cpp](modules/appContext/appContext.cpp)
- **Current Issue**: Calls `configManager.notifyConsumersToReapplyConfig()` while still in the `BOOTING` state.
- **Fix**: Move this call to occur immediately after the transition to `AppState::RUNNING`.

### [Component: systemManager]
#### [MODIFY] [systemManager.cpp](modules/systemManager/systemManager.cpp)
- **Current Issue**: "Board switch detected" fast-path triggers `updateData()` even if the `AppState` is not yet `RUNNING`.
- **Fix**: Add a check for `context->getAppState() == AppState::RUNNING` in the board switch logic.

### [Component: weatherClient]
#### [MODIFY] [weatherClient.cpp](modules/weatherClient/weatherClient.cpp)
- **Current Issue**: Attempts fetches even if no API key is available.
- **Fix**: Add an early return in `updateWeather()` if the `apiKey` string length is zero.

### [Component: rssClient]
#### [MODIFY] [rssClient.cpp](lib/rssClient/rssClient.cpp)
- **Current Issue**: Attempts fetches for empty URLs.
- **Fix**: Add an early return in `executeFetch()` if the URL is empty.

## Verification Plan

### Automated Tests
- Audit serial monitor logs to ensure `APP STATE: RUNNING` appears within seconds of WiFi connection.
- Confirm DNS error logs for RSS/Weather are eliminated during boot.

### Manual Verification
- Verify that the physical display transitions smoothly from the loading screen to the board view.
- Confirm that data populated correctly on the boards after the initial "Loading..." state.
