# Context Bridge: Native Test Restoration

## 📍 Current State & Focus
Recovering the `unit_testing_host` environment after v3.0 refactoring. Currently, the build is blocked by 20+ compilation errors due to interface mismatches (`iDataSource`) and outdated mocks (`appContext`).

## 🎯 Next Immediate Actions
1. **[MODIFY] [appContext.hpp](test/mocks/appContext.hpp)**: Add missing v3.0 accessors (`getDeviceCrypto`, `getAppState`, etc.).
2. **[MODIFY] [Stubs.cpp](test/mocks/Stubs.cpp)**: Align signatures with real `modules/` and standardize on `WifiManager`.
3. **[NEW] [WebServer.h](test/mocks/WebServer.h)**: Create minimal stub to satisfy `timeManager.cpp`.
4. **[MODIFY] [dataManagerTests.cpp](test/test_native/dataManagerTests.cpp)**: Update `MockSource` to use `executeFetch()`.

## 🧠 Decisions & Designs
- **Standardized Mocking**: Mocks will reflect the `modules/` structure exactly to ensure tests are realistic.
- **CamelCase Alignment**: Standardizing on `WifiManager` naming in mocks.
- **Dependency Isolation**: Using a mock header to resolve `WebServer.h` dependency in native host rather than modifying shared library code.

## 🐛 Active Quirks, Bugs & Discoveries
- `timeManager.cpp` inclusion of `WebServer.h` is the main blocker for native compilation.
- `LittleFS` mock in `Stubs.cpp` is missing enough complexity to satisfy `ConfigManager` startup.

## 💻 Commands Reference
- `pio test -e unit_testing_host`

## 🌿 Execution Environment
- Environment: `unit_testing_host` (native)
- Hardware: No hardware required (purely logic tests)
