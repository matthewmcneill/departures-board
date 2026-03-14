# Web Interface Design & User Flow

This document outlines the architectural redesign and user flow for the Departures Board web interface. It incorporates the [Web Codebase Review](file:///Users/mcneillm/.gemini/antigravity/brain/5357ecdd-bebf-4f73-b13b-68cda47f4505/web_codebase_review.md) and defines the roadmap for modernizing the frontend and backend handlers.

## 1. Top-Level User Flow: The Unified Portal

Instead of two separate systems (Captive Portal vs. Config Site), we will adopt a **Unified Single-Page Application (SPA)**. The device will serve the exact same interface whether it is in "Setup Mode" (Access Point) or "Station Mode" (connected to Home WiFi).

### Why the Unified Approach?
- **Cognitive Consistency**: The user sees the same design language, layout, and "tabs" from the first moment they connect.
- **Resilience**: By removing CDN dependencies (Bootstrap/jQuery), we can serve the full "Pro" UI directly from Flash even when the device has no internet access.
- **Less Friction**: No confusing jump between a generic WiFi manager and the "real" departures board settings.

## 2. New Architectural Components

### 2.1 The API Key Registry
Instead of bundling API keys with WiFi or Global settings, we treat them as **Data Providers**.
- **CRUD List**: A centralized registry of providers (National Rail, TfL, OpenWeatherMap).
- **Instructional Cards**: Tapping an unregistered provider opens a card with direct sign-up links and verification logic.
- **Lazy Setup**: Users only configure keys when they add a board that requires that specific data source.

### 2.2 The dynamic Scheduler
The **Scheduler** is a time-driven manager that decides "what shows when".
- **Rules Engine**: "07:00-09:00 -> Focus: Rail", "23:00-06:00 -> Focus: Screensaver".
- **Screensaver as a Board**: The screensaver is just another board type (Black screen + Clock).
- **Manual Override**: Physical button cycles through boards, overriding the scheduler until the next time-based trigger.

## 2. Design Paradigm: "In-situ Diagnostics"

Instead of a single hidden "Diagnostics" page, we will apply the **In-situ Diagnostics** paradigm. Every configuration point should have immediate, conveniently placed feedback to help the user understand the state of that specific component.

### Connectivity Tab
- **Configuration**: SSID, Password.
- **Diagnostics**: Real-time RSSI meter, IP/Gateway info, and the last 5 connection log lines.

### Board Details Card (Rail/Bus/Tube)
- **Configuration**: Station, Filters, Timing.
- **Diagnostics**: 
    - **Sync Status**: "Updated 45s ago" or "Offline: Retry in 30s".
    - **API Feedback**: "Provider: TfL - Status: 200 OK".

### Global System & Health Tab
- **Configuration**: Brightness, Hostname, Reboot.
- **Diagnostics**: **Unified Hardware Health Card**:
    - **Memory**: Live Free Heap sparkline.
    - **Storage**: LittleFS usage percentage.
    - **Identity**: Chip MAC, Firmware Version, NVS Migration status.

---

## 3. Critical Evaluation of the Previous Design
The "Separated" design we currently have exists primarily because:
1. **CDN Reliance**: The legacy UI (Bootstrap 3/jQuery) requires an internet connection to load its styling and logic. Therefore, it *cannot* work in setup mode before WiFi is configured.
2. **Library Constraints**: The `WiFiManager` library is an "all-in-one" solution that provides its own web server and generic UI templates. It's essentially a shortcut that trades aesthetic control for development speed.
3. **Reboot Cycle**: In many ESP32 implementations, changing network modes triggers a hard reset. This makes a "seamless transition" in the browser difficult without a unified state-management system.

**The Refactor Solution**: By moving to Vanilla JS + Pico CSS (under 15KB), we can bundle the entire logic into a single hex-array in Flash, served by our own `WebHandlerManager`, meaning we no longer need the generic `WiFiManager` UI at all.

---

## 3. Configuration Split: Global vs. Board-Specific

To maintain the **Open/Closed Principle (OCP)**, settings must be logically separated. This allows new board types to be added without modifying the global settings UI.

### Board-Specific Options (Detailed Card)
*Options that vary depending on whether the board is Rail, Tube, Bus, or Weather.*

- **Identity**: Station Name/Code (CRS, ATCO, Naptan).
- **Filtering**: Platform filters, service number filters (Bus).
- **Timing**: Time offsets (Arrival/Departure lead times).
- **Navigation**: Secondary/Filter stations (e.g., "Only show services calling at ...").
- **Local Logic**: Board-specific refresh rates (if they differ from global).

### Global / Board-Wide Options
*Settings that affect the physical hardware or system-wide behavior.*

- **Hardware**: OLED Brightness, 180° Flip.
- **Connectivity**: WiFi credentials, Hostname (mDNS).
- **API Registry**: Integrated CRUD for NRE, TfL, OWM keys + instructions.
- **Scheduler**: Set time-based rules for board focus (e.g., Morning Station, Night Screensaver).
- **System**: Firmware Update policy, Timezone, Reboot.
- **Global Features**: RSS Feed URL/Name, Date display toggle on header.

---

## 3. Codebase Review Summary
*Excerpted from [web_codebase_review.md](file:///Users/mcneillm/.gemini/antigravity/brain/5357ecdd-bebf-4f73-b13b-68cda47f4505/web_codebase_review.md)*

### Architectural Issues
- **Weak Encapsulation**: `WebHandlers.hpp` is a monolithic implementation header.
- **Tight Coupling**: Direct reliance on `extern` globals instead of Dependency Injection.
- **Manual Build**: Frontend assets are manually converted to hex arrays.

### Recommended Refactor Roadmap
1. **Extract `WebHandlerManager`**: Move logic into a class with properly injected dependencies.
2. **Frontend Modernization**: Rewrite using Vanilla ES6+ JS and Pico CSS (target < 15KB).
3. **Automated Asset Pipeline**: Use Python scripts to gzip and hexify assets during the build process.
4. **Captive Portal Unification**: Style the `WiFiConfig` portal to match the new UI.

---

## 4. Evaluation of the Proposed Flow
The proposed **WiFi -> Boards -> Global** flow is a significant improvement over the current multi-tab legacy form because:
1. **Scalability**: Adding a "Weather Board" or "News Board" only requires creating one new details card.
2. **Clarity**: Users no longer have to hunt through a long list of checkboxes to find settings for Board #2.
3. **Robustness**: Localizing dependencies (removing CDNs) ensures the UI works even when the device has no internet connection (essential for setup).
