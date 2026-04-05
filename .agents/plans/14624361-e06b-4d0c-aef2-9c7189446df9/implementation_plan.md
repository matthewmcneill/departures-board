# Implementation Plan: Native Test Suite Restoration (v3.0)

Repair and stabilize the `unit_testing_host` environment which was broken by the v3.0 refactor. This plan ensures that the native test harness accurately reflects the new `appContext`, `schedulerManager`, and `iDataSource` architectures.

## Proposed Audit Checklist
- [x] **House Style**: `camelCase` naming for mocks and standard implementation plan headers.
- [x] **Architectural Standards**: Ensures SRP in mocks and alignment with v3.0 Dependency Injection.
- [ ] **UI Design**: N/A (Backend/Logic tests).
- [x] **Resource Impact**: Native host execution; minimal impact on firmware resources.

## User Review Required

> [!IMPORTANT]
> The `appContext` mock is significantly behind the real implementation. I will synchronize it to include `getDeviceCrypto`, `getAppState`, and other missing members required by modern modules.

> [!NOTE]
> `WifiManager` naming is inconsistent across the codebase (WiFiManager vs WifiManager). I will standardize on `WifiManager` (CamelCase) in the mocks to match `modules/wifiManager/wifiManager.hpp`.

> [!WARNING]
> `lib/timeManager/timeManager.cpp` currently fails to compile natively because it hard-includes `WebServer.h` (an ESP32-only header). I will implement a minimal `WebServer.h` mock in `test/mocks/` to satisfy this dependency without modifying core library code if possible, or use a `-DUNIT_TEST` guard in the staging area.

## Proposed Changes

### [Component] Mock Objects (`test/mocks/`)

#### [MODIFY] [appContext.hpp](test/mocks/appContext.hpp)
- Synchronize class definition with `modules/appContext/appContext.hpp`.
- Add missing accessors: `getDeviceCrypto()`, `getOtaUpdater()`, `getWebServer()`, `getAppState()`.
- Add state utility methods: `setFirstLoad()`, `getFirstLoad()`, `updateBootProgress()`, `getBuildTime()`.
- Standardize `getWifiManager()` return type to `WifiManager&`.
- Include `AppState` enum definition.

#### [MODIFY] [Stubs.cpp](test/mocks/Stubs.cpp)
- Update `appContext` method implementations to match the new header.
- Use `schedulerManager schedule` as a member instead of a static local to match real initialization logic.
- Rename `WiFiManager` to `WifiManager` and update `begin()` signature to `begin(const char* hostname)`.
- Implement missing `LittleFS` stubs (e.g., `info()`, `mkdir()`, `rmdir()`) required by `ConfigManager`.
- Provide minimal `Logger::registerSecret` and `Logger::log` implementations.
- Add `AppState` logic.

#### [NEW] [WebServer.h](test/mocks/WebServer.h)
- Create a minimal stub for `WebServer.h` to satisfy `timeManager.cpp` in native builds.

---

### [Component] Native Tests (`test/test_native/`)

#### [MODIFY] [dataManagerTests.cpp](test/test_native/dataManagerTests.cpp)
- Ensure `MockSource` inherits from `iDataSource` (lowercase 'i').
- Update `fetch()` to `executeFetch()`.
- Ensure all virtual methods are implemented to satisfy the compiler.

---

### [Component] Environment Configuration

#### [MODIFY] [platformio.ini](platformio.ini)
- Audit `build_flags` for `unit_testing_host` to ensure all `modules/` paths are included.
- Add `-I test/mocks` as a high-priority include to override system headers.

## Open Questions

- Should I also update `displayManager` tests if they exist? (Initial audit shows only `dataManagerTests.cpp` is currently failing in the baseline run, but others may be discovered once the build passes).

## Verification Plan

### Automated Tests
- `pio test -e unit_testing_host`: Execute the native test suite.
- **Success Criteria**: 100% pass rate for `dataManagerTests`.

### Manual Verification
- Verify that the firmware still builds for `esp32s3nano` to ensure no mock-leaks into the real hardware build.
