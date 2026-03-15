# Web Interface Design & User Flow

This document outlines the architectural redesign and user flow for the Departures Board web interface. It incorporates the [Web Codebase Review](file:///Users/mcneillm/.gemini/antigravity/brain/5357ecdd-bebf-4f73-b13b-68cda47f4505/web_codebase_review.md) and defines the roadmap for modernizing the frontend and backend handlers.

## 1. Top-Level User Flow: The Unified Portal

Instead of two separate systems (Captive Portal vs. Config Site), we will adopt a **Unified Single-Page Application (SPA)**. The device will serve the exact same interface whether it is in "Setup Mode" (Access Point) or "Station Mode" (connected to Home WiFi).

### 1.1 The Bespoke Captive Portal
We will remove the bloated third-party `WiFiManager` library. Instead, we will implement our own lightweight DNS Hijacker (`DNSServer`) alongside our existing `WebHandlerManager`.
- **Zero Bloat**: This removes roughly 30-50KB of compiled HTML/Bootstrap strings from flash memory.
- **Cognitive Consistency**: The user sees the same Pico CSS design language from the first moment they connect.
- **Focus Mode**: When the device boots in AP Mode (Setup Mode), the frontend will detect this state and **hide all tabs except the WiFi tab**. This prevents users from trying to configure boards or fetch APIs when there is no upstream internet connection.
- **Live Scanning**: The WiFi tab will feature an asynchronous network scanner to populate available SSIDs directly into the Pico dropdown, replicating the ease of use of the old third-party library.
- **Automatic Testing & Hand-off**: When a user inputs new credentials, they can click "Test Connection" to perform a dry-run connection attempt while keeping the AP mode alive. The frontend will stream the connection logs. Upon clicking "Save & Apply", the frontend will display clear hand-off instructions ("Reconnecting... please join your home WiFi and visit http://<hostname>.local") as the device reboots.

## 2. Design Paradigm: Modern SPA Utility

The interface will adhere to a **Mobile-First, Low-Overhead** design paradigm inspired by modern app interfaces, specifically utilizing **Pico CSS** for a premium look with minimal bytes.

### 2.1 Key UI Patterns (The "uiproto" standard)
- **Bottom App Navigation**: A persistent fixed footer for instant tab switching (WiFi, Keys, Boards, Schedule, System).
- **Sticky Global Action**: A "Save & Apply" button pinned to the top of the viewport. This reinforces the "Batch Save" model where users can tweak multiple settings before triggering a device write/reboot.
- **Drawer Diagnostics**: Status information (RSSI, API health) is tucked into expandable inline drawers (`.diagnostic-container`). This keeps the UI clean while putting technical data exactly where it's needed.
- **Native Modal CRUD**: Selection and editing of Boards/Keys occurs in native `<dialog>` overlays, preventing navigation jumps.
- **Theme Native**: Built-in support for Light/Dark/Auto modes using Pico's CSS variables.

## 3. New Architectural Components

### 3.1 The API Key Registry
Instead of bundling API keys with WiFi or Global settings, we treat them as **Data Providers**.
- **CRUD List**: A centralized registry of providers (National Rail, TfL, OpenWeatherMap).
- **Instructional Cards**: Tapping an unregistered provider opens a card with direct sign-up links and verification logic.
- **Lazy Setup**: Users only configure keys when they add a board that requires that specific data source.

### 3.2 The Dynamic Scheduler
The **Scheduler** is a time-driven manager that decides "what shows when".
- **Rules Engine**: "07:00-09:00 -> Focus: Rail", "23:00-06:00 -> Focus: Screensaver".
- **Screensaver as a Board**: The screensaver is just another board type (Black screen + Clock).
- **Manual Override**: Physical button cycles through boards, overriding the scheduler until the next time-based trigger.

---

## 4. Detailed Interface Specification

### 4.1 Connectivity Tab
Focus: Getting the device online and keeping it there.

#### Mode Context
The tab reacts to the current network state returned by `/api/status`:
- **AP Mode (Setup)**: Device acts as an Access Point (`192.168.4.1`). All other tabs (Boards, Keys, System) are disabled/hidden. Focus is purely on configuration.
- **Station Mode (Connected)**: Device is connected to the home network. Standard functionality applies.

#### Form Fields & UI Components
| Field/Component | Type | Mapping (C++) | Description |
| :--- | :--- | :--- | :--- |
| **Network State** | Banner | `status.ap_mode` | Visual banner showing "Setup Mode" (Warning color) or "Connected to [SSID]" (Success color). |
| **SSID** | Dropdown + Button | `wifiSsid` | Targeted WiFi network. Populated via `GET /api/wifi/scan`. The currently configured network is prioritized at the top, labeled with a lightning bolt (⚡) and "(Configured)". Includes an async "↻ Rescan" button next to the field. Includes an option for "Hidden Network" that reveals a standard text input. |
| **Password** | Password + Toggle | `wifiPass` | WiFi network password. Includes a standard "eye" icon to unmask text for verification on mobile keyboards. |
| **Hostname** | Text | `config.hostname` | mDNS name (e.g., `departures.local`). Used to generate the AP name (e.g., "Departures-Board") and network address. |

#### Action Buttons
- **Test Connection**: (Visible mostly in AP Mode) Triggers a `POST /api/wifi/test` to attempt a connection to the selected SSID without rebooting. Success/Failure streams to the Diagnostic Drawer.
- **Save & Apply**: Saves credentials to `wifi.json`. If modifying from AP mode, pops a modal with hand-off instructions ("Please switch back to your home WiFi. The board is now rebooting and will be available at http://[hostname].local") before firing the final reboot API.
- **WiFi Reset**: Triggers a `POST /api/wifi/reset` to erase credentials (LittleFS and NVS) and reboot into AP mode. Requires a clear warning in the UI about the hand-off.

#### In-situ Diagnostics (The Drawer)
- **Signal Strength**: Visual RSSI meter (dBm) or "Access Point Mode" indicator.
- **Network Info**: Current IP, Gateway, and Subnet mask.
- **Connection Log**: A scrollable console showing recent WiFi module logs (especially useful during "Test Connection" to see DHCP timeouts or password rejections).

### 4.2 API Key Registry (CRUD List)
Focus: Managing secrets and identity for data sources.

#### **Key Detail Card**
| Field | Mapping | Description |
| :--- | :--- | :--- |
| **Key Name** | `key.label` (new) | Friendly name (e.g., "Home OWM Key"). |
| **Token/Key** | `key.token` | The actual API secret. |
| **Status** | Diagnostic | Results from a "Test Connection" button. |
| **Setup Help** | UI Component | **Instructional Cards**: Pop-up cards or links to help users find where to get their tokens (e.g. National Rail sign-up). |

### 4.3 Board Manager (CRUD List)
Focus: Managing the virtual boards in the carousel.

#### **Board Detail Card: General**
| Field | Mapping | Description |
| :--- | :--- | :--- |
| **Board Name** | `board.name` | Human label (e.g., "Home Station"). |
| **API Key** | `board.apiKeyId` (new) | **Dropdown**: Select from compatible keys in the Registry. |
| **Show Weather** | `board.showWeather` | Toggle weather overlay for this board. |

#### **Board Detail Card: Type Specifics**
- **Rail**: CRS Code, Platform Filter, Calling Station (Secondary ID), Time Offset.
- **Tube & Bus**: Naptan/Atco ID, Line/Route filters.
- **Screensaver**: Style (Analog, Digital, Blank), Brightness Override.

### 4.4 Global System & Health
Focus: Hardware toggles and overall device maintenance.

| Category | Field | Mapping | Description |
| :--- | :--- | :--- | :--- |
| **Hardware** | Brightness | `config.brightness` | PWM level (0-255). |
| **Hardware** | Flip Screen | `config.flipScreen` | Rotate 180 degrees. |
| **System** | Timezone | `config.timezone` | POSIX timezone string. |
| **System** | Update Policy | `config.firmwareUpdatesEnabled` | Background OTA toggle. |
| **Maintenance** | Factory Reset | API Call | Full wipe of `/config.json`, `/apikeys.json`, and `/wifi.json`. |
| **Maintenance** | Reboot | API Call | Dedicated button to restart the board. |
| **RSS Config** | RSS URL | `config.rssUrl` | XML news feed source. |

**In-situ Diagnostics (The "Health Card"):**
- **Memory**: Free Heap, Max Alloc.
- **Storage**: LittleFS used/free space (Primary Health feature).
- **Build Info**: MAC, Version, Config Ver.
- **Diagnostics**: Uptime and advanced metrics tucked in health drawer.

---

## 5. Development Strategy: Parallel Evolution

To minimize risk and allow for iterative testing, we will follow a **Parallel Evolution** strategy.

### 5.1 The `portal/` Directory
- **Concept**: A brand new root folder `/portal` will be created. 
- **Blank Canvas**: All modern SPA assets (HTML, JS, CSS) will reside here, starting with the **uiproto** payload.
- **Side-by-Side Operation**: The ESP32 will serve the legacy UI on `/` and the new UI on `/portal`.

---

## 6. Codebase Review Summary
*Excerpted from [web_codebase_review.md](file:///Users/mcneillm/.gemini/antigravity/brain/5357ecdd-bebf-4f73-b13b-68cda47f4505/web_codebase_review.md)*

### Architectural Issues
- **Weak Encapsulation**: `WebHandlers.hpp` is a monolithic implementation header.
- **Tight Coupling**: Direct reliance on `extern` globals instead of Dependency Injection.

### Recommended Refactor Roadmap
1. **Extract `WebHandlerManager`**: Move logic into a class with properly injected dependencies.
2. **Frontend Modernization**: Rewrite using Vanilla ES6+ JS and Pico CSS (target < 15KB).

---

### Functional Gaps
- **Hidden Networks**: Enhanced "Manual SSID" entry flow within the scanner interface.
- **Screensaver Mode**: Implementation of the "Screensaver" board type as a selectable transport mode.
