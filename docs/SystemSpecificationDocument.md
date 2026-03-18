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
- **Design Philosophy**: **Composition over Inheritance**. UI logic is decoupled from data logic using a plugin-based architecture where `iDisplayBoard` instances are composed of reusable `iGfxWidget` components.
- **Component Diagram**:
    - `appContext` (Orchestrator) $\rightarrow$ `DisplayManager` / `ConfigManager` / `SystemManager` / `WiFiManager`.
    - `DisplayManager` (Carousel) $\rightarrow$ `BoardVariant` (Static Pool) $\rightarrow$ `iDisplayBoard` implementations.
    - `iDisplayBoard` $\rightarrow$ `iDataSource` (Data) + `iGfxWidget`s (UI).
- **Tech Stack**:
    - **Firmware**: C++17, PlatformIO, `ArduinoJson`, `u8g2`, `WiFiClientSecure` (TLS 1.2+).
    - **Web Portal**: Vanilla JS (ES6), Pico CSS, HTML5 (Gzipped).
    - **Memory Strategy**: **Static Allocation for Core Logic**; `std::variant` for board pooling; `std::unique_ptr` for transient network clients.

## 4. Data Design
- **Data Models/Schema**:
    - **`BoardConfig`**: A JSON-serialized structure defining a display board's type (Rail, Bus, Tube), its unique identifier (CRS/Naptan), geographical coordinates (`lat`/`lon`), and localized feature toggles (e.g., `showWeather`).
    - **`rdService`**: A fixed-size C++ struct used for internal service representation, containing destination, timing, status, and mode-specific metadata (e.g., train length or tube line name).
    - **`WeatherStatus`**: An injectable object containing Condition IDs, temperature, and day/night flags, owned per-board to support localized weather.
- **Data Storage**:
    - **Flash (SPIFFS/LittleFS)**: Stores `config.json` (system settings), `apikeys.json` (registry), and `rss.json` (feed list).
    - **Memory (RAM)**:
        - **Static DRAM**: Holds core managers (`appContext`, `DisplayManager`) and the `BoardVariant` pool to ensure zero fragmentation.
        - **System Heap**: Reserved for transient **Network Clients** (`WiFiClientSecure`), **JSON Documents**, and **SSL Buffers**.
        - **Task Stack**: Limited to ~8KB; large local variables are strictly avoided to prevent kernel corruption.
- **Data Flow**:
    1. **Trigger**: `SystemManager` or `Carousel` triggers `updateData()` on the active board's data source, instantly returning a `UPD_PENDING` (9) state while a FreeRTOS background task is spawned.
    2. **Coordinate Recovery**: For National Rail boards, if coordinates are missing from the primary OJP autocomplete picker (which lost spatial data in March 2026), the system performs a fallback lookup via the **TfL Search API** (`api.tfl.gov.uk/StopPoint/Search`) to ensure the weather system remains functional.
    3. **Background Fetch & Parse**: A FreeRTOS task running exclusively on Core 0 performs the non-blocking HTTP GET/POST (REST or SOAP). Streaming parsers populate isolated background variables (`bgStatus`, etc.).
    4. **Mutex Synchronization**: Upon parse completion, the task acquires a `SemaphoreMutex`, safely `memcpy`s the verified structures into the UI-facing active memory blocks (`renderData`), sets `UPD_SUCCESS`, and immediately releases the lock to prevent memory tearing.
    5. **Render**: On the next `tick()`, `iDisplayBoard` reads the valid state and consumes the fresh data via `iGfxWidget`s for final hardware output.
- **Data Retention & Archiving**: Configuration is persistent across reboots; real-time transport and weather data are purged on power loss or board deactivation.

## 5. Detailed Component/Module Design

### 5.1 `appContext` (Central Orchestrator)
- **Role**: Servant as the "System Hub", it owns the lifecycle of all core managers and acts as the primary dependency injection (DI) provider.
- **Architecture**: Implements a **Hierarchical State Machine (HSM)** to manage top-level application states (`BOOTING`, `WIFI_SETUP`, `BOARD_SETUP`, `RUNNING`). 
- **Key Logic**:
    - **Deterministic Boot**: Sequential initialization of managers to avoid static initialization order issues.
    - **Service Discovery**: Provides accessor methods for modules to find peer services (e.g., `getConfigManager()`).
    - **Yield Pattern**: Proxies yield events from data clients to the `DisplayManager` to keep animations fluid during network I/O.

### 5.2 `configManager` (Persistence & Registry)
- **Role**: Handles JSON-based persistence on the LittleFS filesystem, managing both system settings and the API Key Registry.
- **Architecture**: 
    - **`Config` Struct**: A master container for all user preferences and board configurations.
    - **Consumer Registry**: Implements an `iConfigurable` registration pattern, allowing managers (like `DisplayManager`) to be notified when the configuration is updated via the Web UI.
- **Data Integrity**: 
    - Supports **Schema Migration** (e.g., v1.x flat config to v2.x array-based boards).
    - Validates mandatory fields (`CRS`, `Naptan`) before marking a board as `complete`.

### 5.3 `displayManager` (Rendering & Carousel)
- **Role**: Hardware display orchestrator, power manager, and board lifecycle controller.
- **Displays Model (Board Persistent Storage)**:
    - **Static Memory Pool**: Utilizes a type-safe memory pool `std::variant<std::monostate, NationalRailBoard, TfLBoard, BusBoard> slots[MAX_BOARDS]` to pre-allocate memory for all possible board instances. This strictly avoids runtime heap fragmentation and ensures long-term stability.
    - **Polymorphism**: Implements the `iDisplayBoard` interface supporting `onActivate()`, `onDeactivate()`, and `tick()` lifecycle hooks.
    - **System Board Registry**: Maintains singletons for functional screens (Splash, WiFi Wizard, Help, Message, Sleeping).
- **Animation & Rendering Approaches**:
    - **Dual-Path Strategy**:
        - **Logic Path**: Standard 10ms-100ms logic updates via `tick()` preceding a full synchronized `render()` pass.
        - **Yield Path**: High-priority `renderAnimationUpdate()` (60fps target) acting as an "escape hatch" during long-running network operations (e.g., SOAP/WSDL fetches). This path is triggered via a yield wrapper to keep UI elements like scrollers and clocks responsive during blocking I/O.
    - **Hardware Optimization**: Implements direct hardware buffer updates for overlays (e.g., WiFi warnings) using `u8g2.updateDisplayArea()`.
- **Widget-Based Composition**:
    - **Architecture**: Graphical components implement the `iGfxWidget` base class. Complex boards are composed of multiple widgets (e.g., `HeaderWidget`, `ServiceListWidget`, `ScrollingTextWidget`).
    - **Deduplication Pattern**: Widgets calculate state changes during `tick()`, but only trigger expensive hardware SPI transfers if the visual state has actually moved, minimizing SPI bus saturation.
- **Font Integration**: Rendering logic utilizes custom RLE-compressed fonts (Section 11) such as `NatRailTall12` and `NatRailSmall9`, optimizing for both legibility and memory footprint on the ESP32.

### 5.4 `systemManager` (Global Logic & Polling)
- **Role**: Manages non-display tasks including network status monitoring, data refresh timers, and boot progress.
- **Logic**: 
    - **Polling Orchestration**: Calculates the next update window for the active board and coordinates RSS feed updates.
    - **Message Aggregator**: Populates the `globalMessagePool` with headlines and disruption alerts from various sources.
    - **Status Tracking**: Monitors persistent WiFi errors and manages the "Alternate Station" schedule logic.

### 5.5 `weatherClient` (Stateless Integration)
- **Role**: Fetches and parses weather data from the OpenWeatherMap REST API.
- **Architecture**: 
    - **Stateless Design**: The client does not store results internally; it populates a passed **`WeatherStatus`** reference owned by a specific board.
    - **Streaming Parser**: Uses `JsonStreamingParser` (listener pattern) to process payload chunks, significantly reducing the stack/heap memory footprint.
- **Data Representation**: 
    - Stores the **Condition ID (`int`)** and a **Boolean `isNight` flag** for high-efficiency lookup.
    - Uses a custom **`WeatherIcons16`** pixel font mapping condition IDs to ASCII icons.
- **Features**: 
    - Supports per-board localized weather and real-time "Test Feed" token overrides.
    - Implements the `WS_UNCONFIGURED` state (Grey Dot) for immediate user feedback.

### 5.6 `webServer` (Local GUI & API)
- **Role**: Serves the gzipped web portal assets and provides JSON API endpoints (`/api/status`, `/api/saveall`, `/api/test`).
- **Architecture**: 
    - **Hand-off Pattern**: Delegates complex routing and API logic to the `WebHandlerManager`.
    - **Re-entrancy Protection**: Implements a recursion lock in `handleClient()` to prevent stack overflows during deep callback chains.

### 5.8 `iDisplayBoard` (The UI Contract)
- **Role**: Pure abstract interface for all screen implementations.
- **Methods**:
    - `onActivate()` / `onDeactivate()`: Lifecycle hooks for state reset.
    - `render(display)`: Full screen draw during the main loop.
    - `animationTick(display, ms)`: Dedicated hook for **Non-Blocking Animations** during network I/O.
- **Implementations**: Includes all transport boards (NR, TfL, Bus) and System boards (Splash, WiFi Wizard).

### 5.9 `iGfxWidget` (UI Composition)
- **Role**: Reusable graphical components responsible for their own bounding box, state, and rendering.
- **Key Widgets**:
    - **`headerWidget`**: Station title and status icons.
    - **`clockWidget`**: Blinking colon time with partial update support.
    - **`serviceListWidget`**: Dynamic layout for departures/arrivals.
    - **`scrollingMessagePoolWidget`**: Optimized horizontal marquee for disruptions and RSS.

### 5.10 `iDataSource` (The Data Contract)
- **Role**: Abstraction for external API connectivity, separating parsing from presentation.
- **Methods**:
    - `updateData()`: Instantly returns `UPD_PENDING` (9) while delegating network calls to a FreeRTOS background task.
    - `getLastUpdateStatus()`: Polled by Board controllers to track background fetching task progress.
    - `testConnection()`: Lightweight credential validation.
- **Strategy**: Each board owns a specific datasource (e.g., `NationalRailDataSource`) which parses data into an isolated background buffer before yielding across a Thread-Safe Mutex to the active render structure.

### 5.11 `BoardVariant` (Memory Optimization)
- **Role**: A `std::variant`-based static memory pool managed by `DisplayManager`.
- **Logic**: Allocates exactly `MAX_BOARDS` slots of the largest board size at boot. This allows the system to switch between complex board types (Rail vs. Bus) without ever calling `new` or `delete` on the heap during operation, ensuring long-term stability.

## 6. User Interface Design
- **Design Philosophy**: 
    - **Minimalist Footprint**: Avoids heavy frameworks (Bootstrap/jQuery) in favor of **Vanilla ES6+ JS** and **Pico CSS**, keeping the uncompressed payload under 20KB.
    - **Asset delivery**: Static assets are compressed via **gzip** and converted to **PROGMEM hexadecimal arrays**. This allows the server to stream files directly from Flash memory without taxing the LittleFS filesystem or DRAM.
    - **Mobile-First UX**: Prioritizes Fluid Grid layouts (Flexbox/Grid) and "Z-pattern" visual scanning for physical interaction in the field.
- **Navigation & User Flow**: 
    - **Unified Portal**: The same interface is served during initial setup (AP Mode) and normal operation, providing **Cognitive Consistency**.
    - **Contextual Visibility**: In `WIFI_SETUP` mode, non-essential tabs are hidden to guide the user through connectivity first.
    - **Sequential Lifecycle Orchestration**: The portal implements a **`testQueue`** to serialize diagnostic requests (API keys, WiFi, Feeds). This prevents the ESP32's network stack from being overwhelmed by simultaneous SSL handshakes or high-load parsing.

## 7. Security & Authentication
- **Identity Management**: The system relies on local network security. There is no multi-user identity model; access to the portal is granted to any user on the same WiFi network or connected to the device's Access Point.
- **Authorization**: Configuration changes are protected by standard HTTP POST validation; however, no administrative roles are currently implemented.
- **Data Protection**:
    - **Encryption in Transit**: All outbound communication with transport APIs (National Rail, TfL, OWM) utilizes **TLS/SSL**.
    - **API Key Redaction**: The **"Placeholder Pattern"** is used for all secrets (WiFi passwords, API tokens). 
        - The actual secret is **never** sent from the server to the client.
        - The UI renders `placeholder="••••••••"` with a value of `""` (empty string).
        - If the user submits without typing, the backend receives an empty string and preserves the existing secret, preventing accidental erasure.
        - `autocomplete="new-password"` is enforced to prevent browser autofill interference.
- **Vulnerability Mitigation**:
    - **Rate Limiting**: Sequential test execution prevents self-inflicted Denial of Service (DoS) on the ESP32 network stack.
    - **Input Validation**: `ArduinoJson` and strict frontend type-checking mitigate malformed payload injections.

## 8. Infrastructure and Deployment
- **Deployment Architecture**: The system is deployed to **ESP32** microcontrollers (dual-core and single-core variants supported). Flash partitioning typically reserves ~2MB for app code and ~1MB for **LittleFS** (web assets and configuration).
- **CI/CD Pipeline**:
    - **Automated Web Verification**: `run_web_tests.py` executes **Playwright E2E** tests against the portal source. This includes "Mocked API" tests for UI logic and "Live Device" tests for ESP32 parity.
    - **Firmware Unit Testing**: PlatformIO's `native` environment runs **Unity unit tests** on the host machine to verify business logic (e.g., condition code mapping, time string formatting) without hardware dependencies.
    - **Python Tooling**: Custom scripts in `tools/` automate the hashing, minification, and header generation of frontend assets.
- **Environments**:
    - **`esp32dev` / `esp32s3nano`**: Hardware production environments.
    - **`unit_testing_host`**: Local development host for native C++ profiling and logic verification.

## 9. Non-Functional Requirements
- **Performance & Scalability**:
    - **UI Fluidity**: Achieved via a 60Hz loop with localized `animationTick()` updates for smooth scrolling and clock blinking.
    - **Task Concurrency**: All heavy networking and XML/JSON parsing execute entirely within decoupled **FreeRTOS Background Tasks** (`xTaskCreatePinnedToCore`) pinned to Core 0. This guarantees zero blocking on the UI rendering context (Core 1). Native RTOS `vTaskDelay` chunking permits single-core MCUs (ESP32-C3) to seamlessly interweave Wi-Fi interrupts and display updates without processor starvation or Watchdog Timer (WDT) panics.
- **Reliability & Availability**:
    - **Fragmentation Prevention**: The system strictly avoids the `String` class in the "hot path" (API parsing), preferring fixed-size **C-style character arrays**. This ensures the **Max Free Block** size remains sufficient for TLS/SSL handshakes (requiring up to 32KB contiguous RAM).
    - **Stack Protection**: To prevent overflows on the limited 8KB task stack, all heavy network dependencies (Clients, Parsers) are isolated on the heap using **Smart Pointers** (`std::unique_ptr`).
    - **Watchdog Safety**: The Hierarchical State Machine ensures boot-time blocking is kept under the ESP32's 5-second watchdog timer threshold.
- **Observability**:
    - **Serial Telemetry**: Extensive logging via the `Logger` module with severity levels (DEBUG, INFO, WARN, ERROR).
    - **On-Device Health**: The web portal and system boards provide live visualization of Heap usage, LittleFS storage, Chip Temperature, and Uptime.
- **Coding Standards**:
    - Use `snprintf` for all string formatting to ensure null termination and prevent buffer overflows.
    - Avoid `strcpy` in favor of `strncpy` with explicit boundary checks.

## 11. Font Architecture & Build Pipeline
- **Overview**: The project utilizes custom, authentic UK railway and TfL station board fonts, optimized for low-memory OLED displays.
- **The "Round Trip" Pipeline**:
    - **Source**: Authentic font data is stored in `fonts_recovered.h`.
    - **Decompilation**: `font_decompiler.c` extracts binary arrays into `.txt` dumps.
    - **BDF Generation**: `txt_to_bdf.py` converts dumps into standard BDF source files.
    - **C++ Compilation**: `build_fonts.py` uses the `bdfconv` utility to generate the final `fonts.cpp` linked in the firmware.
- **U8G2 Compression Format**:
    - **Global Header**: 23-byte header defining glyph count, bounding box mode, and bit-level RLE run lengths (`b0`, `b1`).
    - **RLE Encoding**: Uses Run-Length Encoding with a Least Significant Bit (LSB) first bitstream. Decoding involves reading `b0` bits for background and `b1` bits for foreground, followed by a stop bit.
- **Editing Workflow**: Developers can modify fonts using **FontForge** or **Fony** by editing the `.bdf` files in `modules/displayManager/fonts/source/` and running the build script to regenerate `fonts.cpp`.

## 12. Appendices & References
- **Async Data Architecture**: [AsyncDataRetrieval.md](file:///Users/mcneillm/Documents/Projects/departures-board/docs/reference/AsyncDataRetrieval.md)
- **State Machine Reference**: [AppContextStateMachine.md](file:///Users/mcneillm/Documents/Projects/departures-board/docs/reference/AppContextStateMachine.md)
- **Memory Strategy**: [MemoryArchitecture.md](file:///Users/mcneillm/Documents/Projects/departures-board/docs/MemoryArchitecture.md)
- **External Documentation**: 
    - [National Rail OpenLDBWS API Wiki](file:///Users/mcneillm/Documents/Projects/departures-board/docs/NationalRailAPIHistory.md)
    - [TfL Unified API Documentation](https://api-portal.tfl.gov.uk/api-details)
