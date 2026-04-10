# Implement Device-Side MCP Server

## Goal

Phased deployment of a lightweight, device-side Model Context Protocol (MCP) server over HTTP on the ESP32 firmware. The server will allow local AI agents (e.g., Antigravity) to interrogate and actuate the hardware. As per your request, Phase 1 introduces pure telemetry and read-only ("get") capabilities. Future phases layer on mutating actions and developer edge operations.

## User Review Required

> [!IMPORTANT]
> **Memory Limitations**: A standard JSON-RPC HTTP POST body can be memory intensive if tools expect huge matrices (like a raw base64 string of a large display). Please verify during Phase 1 testing whether `get_display_buffer` causes heap warnings, and we can investigate RLE or compression.
> **Authorization Security**: Are we placing an authorization token constraint on the `/mcp` route, or assuming the local network is isolated and secure enough for dev endpoints?
> `get_recent_logs` has been explicitly dropped from this scope in favor of local PCP tools.

## Resource Impact Assessment

- **Flash (ROM)**: Anticipated +15KB to +30KB increase. The usage of PROGMEM schema spooling keeps this strictly in Flash and safely within the remaining ~250KB budget prior to the 1.2MB OTA limit.
- **RAM (Heap/Stack)**: Highly constrained risk. Single monolithic `DynamicJsonDocument` allocation is banned. Execution must utilize `ArduinoJson` stream serialization directly to standard Arduino `Print` references.
- **PSRAM**: N/A, assuming no external RAM dependency.
- **Power**: Occasional WiFi spikes during HTTP requests due to baseband transmission. Insignificant on a mains-powered display.
- **Security**: The current proposal operates with unauthenticated HTTP JSON-RPC payloads. Suitable for a secure local subnet, but could expose the board to malicious `reboot_device` or `trigger_ota_check` requests if placed on a public LAN.

## Proposed Changes

---

### Foundations: Core MCP Router
*Architectural Note: To preserve Separation of Concerns and Dependency Inversion, `mcpServer` will NOT be passed the `AsyncWebServer`. HTTP networking will remain strictly encapsulated inside `webServer` (`WebHandlerManager`). Instead, `mcpServer` will expose a transport-agnostic execution method that accepts an input JSON document and writes directly to an Arduino `Print&` stream. This maintains perfect architectural isolation while still leveraging `ESPAsyncWebServer`'s lightweight chunked response streams via polymorphism.*

#### [NEW] [mcpServer.hpp](modules/mcpServer/mcpServer.hpp)
Creates the structure for the MCP logic decoupled from networking.
- Define `McpTool` struct holding PROGMEM pointers to schemas.
- Declare `mcpServer::init()`
- Declare `static void processPayload(const JsonDocument& requestDoc, Print& outputStream);`

#### [NEW] [mcpServer.cpp](modules/mcpServer/mcpServer.cpp)
Implements the JSON-RPC parsing independent of HTTP headers.
- Implement logic to handle the `"method": "tools/list"` JSON-RPC command *(Note: `tools/list` and `tools/call` refer to the string keys in the MCP Protocol payload, not the `tools/` folder in the GitHub repository)*. This logic spools schemas directly to the `outputStream`.
- Implement the `"method": "tools/call"` parameter parsing mapping.

#### [MODIFY] [webHandlerManager.hpp](modules/webServer/webHandlerManager.hpp)
- Add `void handleMcpRequest(AsyncWebServerRequest *request, const String& body);`

#### [MODIFY] [webHandlerManager.cpp](modules/webServer/webHandlerManager.cpp)
- Add the route: `bindPostDynamic("/mcp", &WebHandlerManager::handleMcpRequest);`
- In `handleMcpRequest`, parse the incoming lightweight JSON request, initialize the `AsyncResponseStream`, and call `mcpServer::processPayload(doc, *responseStream)`.

---

### Phase 1: Observability and Telemetry (Getters)
The initial wave of services. Completely safe, read-only interrogations.

#### [MODIFY] [mcpServer.cpp](modules/mcpServer/mcpServer.cpp)
Register the following PROGMEM schemas and callback mappings into the tool registry:

1. `get_system_telemetry`: Ask `appContext` for uptime, free heap, lowest free heap.
2. `get_configuration`: Hook to `configManager` to serialize the current `config.json` state out.
3. `get_display_buffer`: Read the underlying raw buffer bytearray from the `displayManager`.
4. `get_active_data`: Aggregate the currently loaded transit and weather data objects from `dataManager`.
5. `get_network_status`: Read `WiFi.localIP()`, MAC, and current Wi-Fi RSSI signal strength from `wifiManager`.

---

### Phase 2: Agent Skill Development (`device-mcp-client`)
To fully leverage the new onboard MCP server, we will use the `skill-creator` methodology to author a new skill that allows AI agents to interact with the device natively. We will build this skill first and test it against Phase 1, then iteratively update it alongside Phase 3 and Phase 4.

#### [NEW] [.agents/skills/device-mcp-client/SKILL.md](.agents/skills/device-mcp-client/SKILL.md)
- Prompts AI agents on how to interact with the device's JSON-RPC over HTTP endpoints.
- Instructs the AI on standard operational flows (e.g., discovering the IP, modifying configs, simulating data).

#### [NEW] [.agents/skills/device-mcp-client/scripts/discovery.py](.agents/skills/device-mcp-client/scripts/discovery.py)
- A helper script for the agent to automatically extract the device's active IP address. It will query the `pio-manager` MCP server's active log cache using the `searchPattern` tool (e.g., matching typical lines like `[SYSTEM] WiFi connected. IP:` or `[WIFI] Connected successfully. IP:`) or prompt the user if logs are unavailable.

#### [NEW] [.agents/skills/device-mcp-client/scripts/capture_display.py](.agents/skills/device-mcp-client/scripts/capture_display.py)
- A specialized Python script that invokes the ESP32's `get_display_buffer` endpoint, parses the returned underlying screen buffer (e.g., RGB565 representation), and renders a reconstructed `.png` file.
- It will automatically save this generated image directly into the active session's `artifacts/` folder, allowing the AI agent to visually inspect hardware matrix drawings without physical access.

---

### Phase 3: Configuration & Actuation (Mutators)
Introduce capabilities to alter live data.

#### [MODIFY] [mcpServer.cpp](modules/mcpServer/mcpServer.cpp)
Register non-destructive actuation capabilities:

1. `patch_configuration`: Accepts a JSON subset and updates `configManager` fields directly, triggering an internal save.
2. `set_display_brightness`: Calls the `displayManager` dimming handler using a provided `[0-255]` integer argument.
3. `render_debug_message`: Instruct the `displayManager` to temporarily overlay a specific text string from the payload parameter over whatever is currently drawn.
4. `set_calibration_display`: Hook into the existing diagnostic functionality to assert the hardware matrix patterns, allowing an agent to verify LEDs remotely.

---

### Phase 4: Developer Edge & Emulation Operations
The final phase introduces potentially destructive operations or simulation shims specifically geared for AI/Developer testing and edge cases.

#### [MODIFY] [mcpServer.cpp](modules/mcpServer/mcpServer.cpp)
Register simulation and system lifecycle tools:

1. `inject_mock_data`: Force the `dataManager` class to accept an injected JSON payload of departure data, ignoring the scheduler. Brilliant for UI testing rare edge cases.
2. `force_data_refresh`: Force `schedulerManager` to strike API servers globally.
3. `reboot_device`: Call `esp_restart()`.
4. `trigger_ota_check`: Call `otaUpdater` to hit GitHub manually.
5. `simulate_button_event`: Inject button state events directly towards `buttonHandler`.

### Documentation

#### [MODIFY] [SystemSpecificationDocument.md](docs/SystemSpecificationDocument.md)
- Document the new MCP subsystem.
- List the tools and capabilities exposed in each phase (telemetry, system mutators, and emulation hooks).
- Detail the memory constraints and the PROGMEM/Print based architecture that developers must adhere to when extending the MCP Server.

## Verification Plan

### Automated Tests
- Use the `HTTPClient` from a python script to iterate over every `mcp` tool. 
- A tool testing script `.agents/scripts/mcp-test.py` should be implemented to validate that `/mcp` returns correct `JSON-RPC` wrappers.

### Manual Verification
1. Boot the ESP32 and check the local Network IP.
2. Perform a `POST` request to `http://<IP>/mcp` using `curl` with a `"method": "tools/list"` payload.
3. Verify that `ArduinoJson` parses all active tools successfully without causing watchdog (WDT) triggers.
4. Execute `get_system_telemetry` to physically verify that heap levels don't permanently drop with each MCP query (validating RAII on memory bounds inside the network callbacks).
