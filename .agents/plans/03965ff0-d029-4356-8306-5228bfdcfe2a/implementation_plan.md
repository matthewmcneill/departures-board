# RAII and Memory Management Refactor

Resolve "Raw Pointer Allocations and RAII Violations" technical debt by migrating manual memory management (`new`/`delete`) to C++14 RAII paradigms (`std::make_unique` and `std::unique_ptr`). This ensures memory safety across transient execution paths in our continuous 24/7 environment.

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
- *Note: External `ESPAsyncWebServer` string payload references like `request->_tempObject = new String();` which expect raw memory may be isolated.*

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
- **Flash Cost (Zero-overhead)**: Switching to `std::unique_ptr` and `std::make_unique` resolves entirely functionally at compile-time as a zero-cost abstraction—producing identical assembly outputs and demanding negligible binary payload compared to manual `delete` wrappers.
- **Dynamic RAM Stability**: Yields profound stability for 24/7 runtimes enforcing stack ownership limits. Should API polls time out or terminate unexpectedly, native stack unwinding invokes `unique_ptr` destructors without omission, eliminating silent transient heap leaks that previously caused fatal Task Watchdog Timer (TWDT) crashes and fragmentation.

## Verification Plan
### Automated Tests
- Run PIO check locally to confirm parsing and lint checks `pio check` (or use internal linter tools).
### Manual Verification
- Compile and flash firmware manually using `/flash-test`.
- Observe Serial Monitor logs (`/monitor`) inspecting API connectivity loops and web API tests (particularly `WiFiClientSecure` handshakes) to confirm runtime memory stability and behavior parity.
