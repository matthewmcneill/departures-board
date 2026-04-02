# RAII and Memory Management Refactor

[x] Reviewed by house-style-documentation - absolute paths corrected to repository-relative; standard headers aligned.
[x] Reviewed by architectural-refactoring - passed; promotes DIP via smart pointer ownership transfer.
[x] Reviewed by embedded-systems - passed; confirmed zero-overhead abstractions for ESP32.

## Goal Description

Resolve "Raw Pointer Allocations and RAII Violations" technical debt by migrating manual memory management (`new`/`delete`) to C++14 RAII paradigms (`std::make_unique` and `std::unique_ptr`). This ensures memory safety across transient execution paths in our continuous 24/7 environment.

## User Review Required

> [!IMPORTANT]
> **Library Isolation**: `ESPAsyncWebServer` internally relies on raw memory for `request->_tempObject = new String()`. These specific instances must be isolated to avoid breaking library-internal logic that expects raw pointers for cleanup.

> [!WARNING]
> **Ownership Transfer**: Migrating to `std::unique_ptr` in constructors (e.g., `webHandlerManager`) requires callers to use `std::move()`. This is a breaking change for internal API signatures but essential for clear ownership.

## Documentation Standards

- **Explicit Moves**: Every occurrence of `std::move()` MUST be accompanied by a brief comment explaining that ownership is being transferred and the source variable will become `nullptr`. This aligns with our mission to support future developers in a 24/7 firmware context.

## Proposed Changes

---

### App / System Context
#### [MODIFY] [appContext.cpp](modules/appContext/appContext.cpp)
- Refactor dynamic allocation `new buttonHandler(BUTTON_PIN)` to use `std::make_unique<buttonHandler>` and store it safely.
#### [MODIFY] [systemManager.cpp](modules/systemManager/systemManager.cpp)
- Remove `delete inputDevice;` statements, transferring ownership internally to smart pointers.

---

### Web Services
#### [MODIFY] [webServer.cpp](modules/webServer/webServer.cpp)
- Convert `new WebHandlerManager(...)` invocation to utilize `std::make_unique`.
#### [MODIFY] [webHandlerManager.cpp](modules/webServer/webHandlerManager.cpp)
- Migrate `ApiTestParams` and `ApiTestDataSource` transient pointer allocations to use `std::make_unique`.
- Update API object constructors to accept `std::unique_ptr<ApiTestParams>` and pass dependencies via `std::move()`.
- Eliminate all manual `delete testSource;` and `delete params;` memory deallocations.

---

### WiFi & Weather Clients
#### [MODIFY] [wifiManager.cpp](modules/wifiManager/wifiManager.cpp)
- Refactor `dnsServer = new DNSServer();` to `std::make_unique` eliminating explicit `delete dnsServer;`.
#### [MODIFY] [weatherClient.cpp](modules/weatherClient/weatherClient.cpp)
- Modernize C++11 standard raw allocations `std::unique_ptr<T>(new (std::nothrow) T())` directly to C++14 `std::make_unique<T>()`.

---

### Display Manager & Data Sources
#### [MODIFY] [nationalRailDataSource.cpp](modules/displayManager/boards/nationalRailBoard/nationalRailDataSource.cpp)
- Replace transient instances of `WiFiClientSecure`, `HTTPRequest`, and `xStation` payloads with `std::make_unique`.
- Completely purge procedural `delete httpsClient`, `delete xStation`, and `delete client` commands mitigating risk on premature network exits.
#### [MODIFY] [splashBoard.cpp](modules/displayManager/boards/systemBoard/splashBoard.cpp)
- Overhaul `splashLogo = new imageWidget(...)` into a managed `std::unique_ptr`.
#### [MODIFY] [diagnosticBoard.cpp](modules/displayManager/boards/systemBoard/diagnosticBoard.cpp)
- Refine `activeLayout = new layoutTestDiagnostic(...)` allocation.
#### [MODIFY] [busBoard.cpp](modules/displayManager/boards/busBoard/busBoard.cpp)
#### [MODIFY] [tflBoard.cpp](modules/displayManager/boards/tflBoard/tflBoard.cpp)
#### [MODIFY] [nationalRailBoard.cpp](modules/displayManager/boards/nationalRailBoard/nationalRailBoard.cpp)
- Remove `delete activeLayout;` usages by altering domain property definitions to `std::unique_ptr`.

## Resource Impact Assessment

### Memory (Flash/RAM/Stack/Heap)
- **Flash Cost (Zero-overhead)**: Switching to `std::unique_ptr` and `std::make_unique` resolves entirely functionally at compile-time as a zero-cost abstraction.
- **Dynamic RAM Stability**: Yields profound stability for 24/7 runtimes by enforcing stack ownership limits. Native stack unwinding invokes `unique_ptr` destructors without omission, eliminating silent transient heap leaks that previously caused fatal Task Watchdog Timer (TWDT) crashes.

### Security & Power
- **Security**: Reduces risk of "Use-After-Free" and "Double-Free" vulnerabilities in network-exposed handlers.
- **Power**: Negligible impact; improved stability prevents unplanned reboots which are power-intensive.

## Verification Plan

### Automated Tests
- Run PIO check locally to confirm parsing and lint checks: `pio check`
- Validate build stability: `pio run -e esp32dev`

### Manual Verification
- Compile and flash firmware manually using `/flash-test`.
- Observe Serial Monitor logs (`/monitor`) inspecting API connectivity loops and web API tests to confirm runtime memory stability and behavior parity.
