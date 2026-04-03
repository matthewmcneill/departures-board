# Dependency Injection and Encapsulation Architecture Refactor

## Goal Description
Refactor the `departures-board` codebase to eliminate floating global instances (`displayManager`, `webServer`, `server`, `currentWeather`) and properly route Dependency Injection through the central `appContext`. Furthermore, relocate orphaned callbacks currently residing in `systemManager` (`updateCurrentWeather`, `tflCallback`, `raildataCallback`) out to their appropriate domain owners, adhering to solid OOP encapsulation and the project's house style.

## Proposed Changes

### Domain Logic & Callbacks Relocation
#### [MODIFY] modules/systemManager/systemManager.hpp
- Delete `tflCallback()`, `raildataCallback()`, `updateCurrentWeather()` from the interface as they bleed UI/Data logic into the global state manager.

#### [MODIFY] modules/systemManager/systemManager.cpp
- Remove the implementations of `tflCallback()`, `raildataCallback()`, `updateCurrentWeather()`.

#### [MODIFY] modules/appContext/appContext.hpp
- Relocate the global wrappers (`yieldCallbackWrapper` and `raildataYieldWrapper`) internals directly into `appContext` implementation or their callers, bypassing `systemManager`.

### Configuration and DI Hub
#### [MODIFY] modules/appContext/appContext.cpp
- Update the implementations of the yielding wrappers to use `appContext` directly instead of referencing `sysManager.tflCallback()`.
- Update network logic referencing global `webServer` calls to `getWebServer()`.

### Displays and Memory
#### [MODIFY] modules/displayManager/displayManager.hpp
- Delete `extern DisplayManager displayManager;` entirely.

#### [MODIFY] modules/displayManager/widgets/scrollingTextWidget.cpp
- Remove `extern DisplayManager displayManager;` unneeded declaration.

### Networking and Web Handlers
#### [MODIFY] modules/webServer/webServer.hpp
- Delete `extern WebServerManager webServer;` and `extern AsyncWebServer server;` definitions.
- Remove `void updateCurrentWeather(float latitude, float longitude);` global declaration.
- Provide `AsyncWebServer& getServer()` getter inside `WebServerManager`.

#### [MODIFY] modules/webServer/webServer.cpp
- Replace `AsyncWebServer server(80);` and `WebServerManager webServer;` globals with encapsulated private class members. `WebServerManager` instance is owned by `appContext`. `AsyncWebServer` will become a member: `AsyncWebServer _server{80};`.

### Weather and State
#### [MODIFY] modules/weatherClient/weatherClient.hpp
- Delete `extern weatherClient* currentWeather;`
- Optionally add the `updateCurrentWeather(float lat, float lon)` from `systemManager` directly directly as a member of `weatherClient` if we want to preserve this helper.

## Verification Plan
### Automated Tests
- Run `platformio run` to compile the firmware successfully. It must compile with zero global variable linker errors.
### Manual Verification
- Deploy firmware.
- Verify `appContext` initializes properly and the user sees the WebServer successfully start on port 80.
- Verify Weather API fetching connects and parses without crashes or `nullptr` access.
- Ensure Boot progress screen correctly updates utilizing the refactored progress callbacks without hanging.
