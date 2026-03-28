# Refactoring Input Dependency Injection

- [x] Move `iButton.hpp` and `touchSensor.hpp/cpp` to `lib/` directory
- [x] Refactor `systemManager.hpp` & `systemManager.cpp` to decouple hardware instantiation
- [x] Integrate standard Doxygen documentation for House Style compliance across `systemManager` and `iButton`.
- [x] Verify total decoupling of `touchSensor` by building all endpoints.

## Phase 3: B2.3-W3.0 Upstream Porting
- [x] **1. National Rail Fix:** Modify `nationalRailDataSource.cpp` loop limits and `WiFiClientSecure` connection timeouts.
- [x] **2. Dependency Definitions:** Append `mathieucarbou/ESPAsyncWebServer` to `platformio.ini`.
- [x] **3. WebServer Switch:** Replace synchronous `WebServer` with `ESPAsyncWebServer` inside `webServer.hpp/.cpp`.
- [x] **4. WebHandler Refactoring:** Transition all endpoint routing delegates in `webHandlerManager` to use `AsyncWebServerRequest` structures.
- [x] **5. Verification:** Ensure all endpoints compile seamlessly using `pio run` across both `esp32dev` and `esp32s3nano`.
