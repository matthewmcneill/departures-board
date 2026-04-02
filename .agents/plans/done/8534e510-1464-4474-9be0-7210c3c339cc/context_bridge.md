# Context Bridge

## 📍 Current State & Focus
The project has successfully completed the "DisplayManager Factory Architecture Refactor". The display manager has been decoupled from concrete board implementations, using a factory pattern (`boardFactory.hpp`/`cpp`) and dynamic heap allocation rather than static `std::variant` arrays. This saves permanent static RAM and prevents permanent heap fragmentation. The `nationalRailBoard` UI code has also been fully decoupled from the WSDL network initialization `WiFi.status()` logic, deferring it safely to the background `DataManager` task on Core 0. The final code has been committed locally, and the build passes cleanly (`pio run -e esp32dev` is completely stable).

## 🎯 Next Immediate Actions
- Execute `/plan-queue` if you wish to prepare a new plan for execution, or begin drafting a new implementation plan based on remaining technical debt.
- No immediate outstanding compilation errors exist.

## 🧠 Decisions & Designs
- **Dynamic Board Initialization (Factory Pattern)**: `iDisplayBoard` derivatives are allocated via `new` in `BoardFactory::createSystemBoard` and `BoardFactory::createDisplayBoard`.
- **Heap Fragmentation Assurance**: Active data boards are kept alive while rotating; system boards are created strictly on-demand during boot/wifi setup loops and promptly `delete`d upon teardown. This architectural design yields "Zero Steady-State Fragmentation."
- **WSDL Discovery Backgrounding**: National Rail SOAP initialization checks now safely reside within `nationalRailDataSource::executeFetch()`, leveraging the FreeRTOS background dispatch array to circumvent premature UI locking without requiring implicit `WiFi` include cascades into UI headers.
- **House Style Assurance**: Guaranteed documentation standards by appending mandatory Doxygen headers for the `boardFactory` interface elements.

## 🐛 Active Quirks, Bugs & Discoveries
- No bugs identified.

## 💻 Commands Reference
- Build the binary natively: `pio run -e esp32dev`
- Flash firmware and view logs: `pio run -e esp32dev -t upload -t monitor`

## 🌿 Execution Environment
- **Branch**: `refactor/technical-debt`
- **Compiler**: PlatformIO via `esp32dev` framework.
