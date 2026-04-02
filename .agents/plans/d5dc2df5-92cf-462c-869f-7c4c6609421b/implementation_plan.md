# Goal Description

Perform a comprehensive House Style Documentation pass across the `departures-board` project, specifically targeting the core managers, to ensure 100% compliance with the `house-style-docs` standards without altering any underlying business logic.

## User Review Required

There are no breaking changes or critical logic modifications. This relies purely on commenting non-compliant variables and function declarations.

## Proposed Changes

### System Manager

#### [MODIFY] [systemManager.hpp](modules/systemManager/systemManager.hpp)
- Add same-line trailing comments to `std::function` variables (`onBootProgress`, `onSoftReset`).
- Add Doxygen style `@brief`, `@param`, and `@return` comments for setter injection methods:
  - `setBootProgressCallback`
  - `setSoftResetCallback`
  - `setInputDevice`
- Add Doxygen style `@brief` comments for all data accessors and setters:
  - `getWifiConnected`
  - `setWifiConnected`
  - `getFirstLoad`
  - `setFirstLoad`
  - `getStartupProgressPercent`
  - `setStartupProgressPercent`
  - `getNextRoundRobinUpdate`
  - `setNextRoundRobinUpdate`
  - `getLastDataLoadTime`
  - `getDataLoadSuccess`
  - `getDataLoadFailure`
  - `getLastLoadFailure`
  - `getLastUpdateResult`
  - `getMyUrl`

---

### Web Server

#### [MODIFY] [webServer.hpp](modules/webServer/webServer.hpp)
- Add a same-line descriptive comment to the internal property `WebHandlerManager* _handlerManager`.
- Add same-line descriptive comments to the exported external globals:
  - `extern WebServerManager webServer;`
  - `extern AsyncWebServer server;`
- Add Doxygen style `@brief` and `@param` comments for the exported function:
  - `void updateCurrentWeather(float latitude, float longitude);`

---

### Config Manager

#### [MODIFY] [configManager.hpp](modules/configManager/configManager.hpp)
- Add same-line descriptions for properties of `ScheduleRule` struct (`startHour`, `startMinute`, `endHour`, `endMinute`).
- Convert the malformed inline `@brief` doc for `reloadPending` to a standard same-line trailing comment.
- Add Doxygen style `@brief` and `@return` comments for:
  - `void requestReload()`
  - `bool checkAndClearReload()`

---

### Main Core

#### [MODIFY] [departuresBoard.cpp](src/departuresBoard.cpp)
- Add a same-line comment explanation on initialization of `appContext appContext;` 

---

## Verification Plan

### Automated Tests
- Run PlatformIO build command to verify that no functional logic was broken and that the code compiles cleanly: `pio run -e esp32dev` (or fallback to relying on CI execution).

### Manual Verification
- Review the diff generated across the four modified files to ensure comment format strictly aligns to the `@.agents/skills/house-style-docs/SKILL.md` template.
