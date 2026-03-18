# System Specification Document

## 1. Introduction
- **Purpose**: This document provides a comprehensive technical specification of the Departures Board system, detailing its firmware architecture, web-based configuration portal, and external data integration layers.
- **Scope**: Covers the ESP32 C++ firmware, the Vanilla JS Single Page Application (SPA), and the communication protocols between them.
- **Audience**: Future developers, project maintainers, and hardware integrators.
- **Glossary**:
    - **`appContext`**: The central orchestrator and dependency injection hub.
    - **`iDisplayBoard`**: The polymorphic interface for all display-capable modules.
    - **CRS**: Computer Reservation System code (3-letter code for UK rail stations).
    - **Naptan**: National Public Transport Access Node (unique identifier for UK transport stops).
    - **Carousel**: The rotation logic that cycles through configured display boards.

## 2. System Overview
- **High-Level Description**: The Departures Board is an ESP32-powered IoT device designed to provide at-a-glance, real-time transport information. It features a carousel system that rotates through various "boards" (National Rail, TfL Underground, Bus, Weather, RSS) rendering high-contrast graphics on an OLED display via the `u8g2` library.
- **System Context**: The device acts as a client to various public and private REST/SOAP APIs over WiFi. Configuration is managed via an on-device web portal served directly from the ESP32.
- **Assumptions and Constraints**:
    - **Hardware**: Targets ESP32-based microcontrollers (e.g., ESP32-S3, ESP32-DevKit).
    - **Stability**: Must achieve months of uptime; relies on zero dynamic heap allocation for core data structures to prevent fragmentation.
    - **Connectivity**: Requires a persistent 2.4GHz WiFi connection for real-time updates.

## 3. System Architecture
- **Architectural Pattern**: **Centralized Orchestration with Hierarchical State Machine (HSM)**. The system avoids global state by using a central `appContext` that manages the lifecycle and inter-dependency of all system managers.
- **Component Diagram**:
    - `appContext` $\rightarrow$ `DisplayManager` / `ConfigManager` / `SystemManager` / `WiFiManager`.
    - `DisplayManager` $\rightarrow$ `iDisplayBoard` (Carousel) $\rightarrow$ Specific Board Implementations.
- **Tech Stack**:
    - **Firmware**: C++17, PlatformIO, `ArduinoJson`, `u8g2`, `WiFiManager`.
    - **Web Portal**: Vanilla JS (ES6), CSS3 (Modern UI), HTML5 (served as gzipped hex headers).
    - **Build System**: Python-based `portalBuilder.py` for frontend asset embedding.

## 4. Data Design
- **Data Models/Schema**:
    - **`BoardConfig`**: A JSON-serialized structure defining a display board's type (Rail, Bus, Tube), its unique identifier (CRS/Naptan), geographical coordinates (`lat`/`lon`), and localized feature toggles (e.g., `showWeather`).
    - **`rdService`**: A fixed-size C++ struct used for internal service representation, containing destination, timing, status, and mode-specific metadata (e.g., train length or tube line name).
    - **`WeatherStatus`**: An injectable object containing Condition IDs, temperature, and day/night flags, owned per-board to support localized weather.
- **Data Storage**:
    - **Flash (SPIFFS/LittleFS)**: Stores `config.json` (system settings), `apikeys.json` (registry), and `rss.json` (feed list).
    - **Memory (RAM)**: Transport and weather data are transient and held in statically allocated memory pools (e.g., `std::variant<...>[MAX_BOARDS]`) to prevent heap fragmentation.
- **Data Flow**:
    1. **Trigger**: `SystemManager` or `Carousel` triggers `updateData()` on the active board's data source.
    2. **Fetch**: `iDataSource` performs non-blocking HTTP GET/POST (REST or SOAP).
    3. **Parse**: `ArduinoJson` or custom XML listeners populate memory-mapped structs.
    4. **Render**: `iDisplayBoard` consumes the data and delegates to `iGfxWidget`s for screen output.
- **Data Retention & Archiving**: Configuration is persistent across reboots; real-time transport and weather data are purged on power loss or board deactivation.

## 5. Detailed Component/Module Design

### `appContext` (Central Hub)
- **Purpose**: Manages the lifecycle of all system managers and acts as the Dependency Injection registry.
- **Interfaces**: Provides `get` methods for `ConfigManager`, `DisplayManager`, `WiFiManager`, etc.
- **Logic**: Implements the Top-Level State Machine (`BOOTING` -> `WIFI_SETUP` -> `RUNNING`).

### `DisplayManager` (Graphics Pipeline)
- **Purpose**: Orchestrates the `u8g2` rendering loop and the `Carousel` logic.
- **Logic**: Handles the non-blocking **Yield Mechanism**, allowing background network tasks to trigger localized UI updates (clocks, tickers) via `animationTick()`.

### `iDisplayBoard` (Polymorphic UI Contract)
- **Purpose**: Abstract base class for all screens.
- **Key Methods**: `render(display)`, `tick(ms)`, `onActivate()`, `onDeactivate()`.
- **Implementations**: `NationalRailBoard`, `TfLBoard`, `BusBoard`, `WeatherBoard`, `RSSBoard`, `SystemBoard`.

### `iDataSource` (API Integration Contract)
- **Purpose**: Abstract base class for fetching and parsing external data.
- **Implementations**: 
    - **`NationalRailDataSource`**: SOAP/XML client utilizing the Darwin OpenLDBWS API.
    - **`TfLDataSource`**: JSON client for Underground arrivals.
    - **`WeatherClient`**: Stateless service for OpenWeatherMap integration.
    - **`RSSReader`**: Concurrent-safe news feed aggregator.

### `iGfxWidget` (Composition UI Components)
- **Purpose**: Reusable UI elements (Header, Clock, Service List, Scrolling Ticker).
- **Logic**: Maintains its own internal animation state and geometry, enabling a "plug-and-play" board layout system.

## 6. User Interface Design
- **UI Architecture**: A **Vanilla JS Single Page Application (SPA)** utilizing **Pico CSS** for a lightweight, mobile-first experience. The frontend is state-driven, dynamically rendering components based on real-time hardware status polled from the `/api/status` endpoint.
- **Wireframes/Mockups**: The design follows the **"uiproto"** standard, featuring:
    - **Bottom App Navigation**: Persistent fixed footer for rapid tab switching.
    - **Sticky Global Action**: A "Save & Apply" button pinned to the header for batch updates.
    - **Drawer Diagnostics**: Expandable `.diagnostic-container` elements for detailed status without UI clutter.
- **Navigation & User Flow**: 
    - **Unified Portal**: The same interface is served during initial setup (AP Mode) and normal operation.
    - **Contextual Visibility**: In `WIFI_SETUP` mode, non-essential tabs are hidden to guide the user through connectivity first.
    - **Sequential Validation**: The portal implements a `testQueue` to serialize diagnostic requests, preventing hardware saturation during SSL handshakes.

## 7. Security & Authentication
- **Identity Management**: The system relies on local network security. There is no multi-user identity model; access to the portal is granted to any user on the same WiFi network or connected to the device's Access Point.
- **Authorization**: Configuration changes are protected by standard HTTP POST validation; however, no administrative roles are currently implemented.
- **Data Protection**:
    - **Encryption in Transit**: All outbound communication with transport APIs (National Rail, TfL, OWM) utilizes **TLS/SSL**.
    - **API Key Redaction**: The **"Placeholder Pattern"** is used for all secrets (WiFi passwords, API tokens). The actual secret is never sent to the browser; the UI displays a masked placeholder (`••••••••`), and the backend ignores empty or unchanged submissions to prevent accidental erasure.
- **Vulnerability Mitigation**:
    - **Rate Limiting**: Sequential test execution prevents self-inflicted Denial of Service (DoS) on the ESP32 network stack.
    - **Input Validation**: `ArduinoJson` and strict frontend type-checking mitigate malformed payload injections.

## 8. Infrastructure and Deployment
- **Deployment Architecture**: The system is deployed to **ESP32** microcontrollers (dual-core and single-core variants supported). Flash partitioning typically reserves ~2MB for app code and ~1MB for **LittleFS** (web assets and configuration).
- **CI/CD Pipeline**:
    - **Web Validation**: `run_web_tests.py` automatically executes **Playwright E2E** tests against the portal source before every build.
    - **Firmware Validation**: PlatformIO's `native` environment runs **Unity unit tests** on the host machine to verify hardware-agnostic logic.
- **Environments**:
    - **`esp32dev` / `esp32s3nano`**: Hardware production environments.
    - **`unit_testing_host`**: Local development host for native C++ profiling and logic verification.

## 9. Non-Functional Requirements
- **Performance & Scalability**:
    - **UI Fluidity**: Achieved via a 60Hz loop with localized `animationTick()` updates for smooth scrolling and clock blinking.
    - **Task Concurrency**: Non-blocking **Yield Callbacks** during network I/O ensure the UI remains responsive while handling large payloads (e.g., National Rail SOAP).
- **Reliability & Availability**:
    - **Heap Stability**: Core managers use static DRAM allocation. Transient operations (SSL/JSON) use heap isolation with `std::unique_ptr` to prevent long-term fragmentation.
    - **Watchdog Safety**: The Hierarchical State Machine ensures boot-time blocking is kept under the ESP32's 5-second watchdog timer threshold.
- **Observability**:
    - **Serial Telemetry**: Extensive logging via the `Logger` module with severity levels (DEBUG, INFO, WARN, ERROR).
    - **On-Device Health**: The web portal and system boards provide live visualization of Heap usage, LittleFS storage, Chip Temperature, and Uptime.

## 10. Appendices & References
- **Historical Session Mapping**: [Detailed Mapping of 100+ Sessions](file:///Users/mcneillm/Documents/Projects/departures-board/docs/HistoricalSessionMapping.md)
- **State Machine Reference**: [AppContextStateMachine.md](file:///Users/mcneillm/Documents/Projects/departures-board/docs/AppContextStateMachine.md)
- **Memory Strategy**: [MemoryArchitecture.md](file:///Users/mcneillm/Documents/Projects/departures-board/docs/MemoryArchitecture.md)
- **External Documentation**: 
    - [National Rail OpenLDBWS API Wiki](file:///Users/mcneillm/Documents/Projects/departures-board/docs/NationalRailAPIHistory.md)
    - [TfL Unified API Documentation](https://api-portal.tfl.gov.uk/api-details)
