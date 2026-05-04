# Goal Description

The current `[env:unit_testing_host]` environment fails to compile because standard ESP32 headers (`HardwareSerial.h`, `WebServer.h`, `WiFi.h`) bleed into native x86/ARM host execution through direct `#include` statements. The manual mocking attempts inside `test/mocks/Stubs.cpp` have deteriorated and are producing dozens of compiler errors. 

The goal of this revision is to modernize our testing pipeline by substituting the fragile manual stubs with `ArduinoFake` (the industry standard for PlatformIO host mocking), introducing strict dependency inversion for complex network objects, and utilizing `lib_ignore` to block raw hardware code from the test compiler.

## Hardware Dependency Evaluation

Based on an in-depth `grep` audit of the repository, the following library boundaries have been identified alongside their mock resolution strategies:

### ✅ Covered by `ArduinoFake`
These standard libraries and primitives will be intercepted natively by `ArduinoFake`.
- `<Arduino.h>` (String, Serial, delay, millis, digitalRead/Write)
- `<HardwareSerial.h>` (Used in `logger.cpp`)
- Standard SPI / I2C stubs where declared.

### ⚠️ Requires Interface Redefinitions or `lib_ignore`
`ArduinoFake` does **not** mock raw ESP32 networking or file system libraries. These must be abstracted behind C++ Interfaces (HAL) or strictly excluded from native compilation. 

- `<WiFi.h>` (Heavily used in `wifiManager.cpp`, `appContext.cpp`, `timeManager.cpp`). 
- `<WebServer.h>` and `ESPAsyncWebServer` (Used in `webServer.cpp`, leaking into `timeManager.cpp`).
- `<LittleFS.h>` (Used in `configManager.cpp`, `webServer.cpp`).
- `<HTTPClient.h>` / `NetworkClientSecure` (Used in `nrRDMDataProvider.cpp`, `busDataSource.cpp`).

---

## Proposed Changes

### Configuration Layer

#### [MODIFY] `platformio.ini`
- **Add Dependency:** Inject `fabiobatsilva/ArduinoFake@^0.4.0` to the `lib_deps` of `[env:unit_testing_host]`.
- **Set Flags:** Ensure `build_flags = -std=gnu++17` is cleanly applied.
- **Isolate Code:** Add sweeping `lib_ignore` or `test_filter` rules. Purely physical managers like `webServer` or `wifiManager` should be excluded from native compilation.

---

### Mocks & Stubs Subsystem

#### [MODIFY] `test/mocks/Stubs.cpp` & `test/mocks/appContext.hpp`
- **Delete Manual Fakes:** Erase the broken implementations of `appContext`, `WiFiManager`, etc.
- **Implement ArduinoFake:** Include `<ArduinoFake.h>` and use GoogleMock assertions (`When(Method(ArduinoFake(), Serial))...`) to satisfy the basic `logger.cpp` outputs during test runs.
- **WASM Compatibility Guard (`__EMSCRIPTEN__`):** The `test/mocks/` directory is shared with the WASM Layout Emulator (`tools/layoutsim/`). Any inclusion of `ArduinoFake` (which pulls in GoogleMock) must be strictly guarded by `#ifndef __EMSCRIPTEN__` to prevent bloating or crashing the web emulator build process.
- **Signature Alignment:** Add missing v3.0 accessors (`getDeviceCrypto`, `getAppState`, etc.) to `appContext.hpp` to clear the legacy `test_native` compilation errors.

#### [NEW] `test/mocks/WebServer.h`
- Create a minimal stub header to satisfy `timeManager.cpp`. This was identified in a prior native-test plan attempt (Plan `14624361`). Since `timeManager` handles core logic, it cannot be excluded via `lib_ignore`, so an empty or mocked `WebServer` definition must be injected into the test path.

#### [NEW] `test/mocks/MockFileSystem.hpp` & `.cpp`
- **Native Subsystem:** Rather than using abstract GoogleMocks, we will build a Native Implementation of `LittleFS` that wraps standard POSIX `<fstream>` and `<dirent.h>`. 
- **Folder Mapping:** This layer will seamlessly intercept `LittleFS.open()` requests and redirect them to write physical files inside `.agents/tmp/native_fs/`. This ensures `configManager` and other JSON parsers are tested against real disk I/O, identical to how flash memory would behave.

---

### Core Business Logic Decoupling

#### [MODIFY] `test/test_native/dataManagerTests.cpp`
- **Resolve Abstract Class Error:** Update `MockSource` to use the correct `executeFetch()` / `serializeData()` overrides as identified in prior testing logs.

#### [MODIFY] `lib/logger/logger.cpp`
- **Fix Input:** Wrap physical serial declarations with `#ifndef UNIT_TEST` guards. Import `<ArduinoFake.h>` appropriately so the native compiler intercepts the `Serial.println()` queues.

#### [MODIFY] `lib/timeManager/timeManager.cpp`
- **Remove Server Leak:** `timeManager` mysteriously includes `<WebServer.h>`. Ensure the new `test/mocks/WebServer.h` intercept satisfies this requirement without crashing the native tester.
- **Stub NTP:** Implement an interface or pre/post compiler flag structure for `configTime` and NTP synchronization logic.

#### [MODIFY] `modules/dataManager/iDataSource.hpp`
- **Interface Repair:** Repair the `MockSource` declared in the test suite to ensure it fulfills the correct pure virtual overrides (like `serializeData`).

## Verification Plan

### Automated Tests
- Command: `mcp_platformio_clean_project` followed by `mcp_platformio_run_tests` targeting `native`.
- Success Criteria: The unit test suite completely compiles on the developer machine (macOS/Linux) in seconds and executes cleanly without any `ModuleNotFoundError` or C++ out-of-line class definition failures.

### Manual Verification
- Command: `mcp_platformio_build_project` targeting `esp32dev` to mathematically ensure that introducing `ArduinoFake` to the native environment did not mutate or break the raw firmware compilation.
