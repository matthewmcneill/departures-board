# On-Device Model Context Protocol (MCP) Server & Memory Management

This document explains the architecture, memory management strategies, and operational usage of the on-device MCP server running on the Departures Board firmware.

## 1. Role in Agentic Debugging

In the Departures Board project, the ESP32 is not just a display controller but a **locally-aware agent node**. Because AI agents (like Antigravity) cannot physically see the device or feel its hardware state, the **On-Device MCP Server** provides the necessary sensory input and actuation capabilities.

By exposing a standard JSON-RPC 2.0 interface over the local network, the agent can:
- **Visualize the Display**: Capture raw framebuffers and decode them into images.
- **Audit Telemetry**: Monitor heap health, temperature, and network stability in real-time.
- **Inspect Configuration**: Read and modify system settings without requiring a serial connection.
- **Probe Live Data**: Inspect the current transit and weather data caches being used by the UI.

## 2. Architecture Overview

 The MCP server is implemented in the `mcpServer` module (`modules/mcpServer/`). 

### Protocol: JSON-RPC 2.0
The server adheres to the [Model Context Protocol](https://modelcontextprotocol.io) using a stateless HTTP Post transport.

- **Endpoint**: `POST /mcp`
- **Request Format**: Standard JSON-RPC 2.0 `tools/list` or `tools/call`.

### Key Components:
- **The Registry**: A collection of tools defined using `PROGMEM` schemas to keep RAM usage flat.
- **Safe Dispatcher**: Incoming calls are routed through `appContext`, ensuring that hardware access is thread-safe and respects the device state machine.
- **Streaming Encoder**: Large responses (like display buffers) are encoded and streamed directly to the network buffer in chunks to prevent heap exhaustion.

---

## 3. Lean Memory Approach

One of the most critical aspects of this project is maintaining a stable **1.2MB firmware footprint** (required for OTA support) while handling complex SSL handshakes and JSON parsing.

### Static DRAM vs. RAII Heap
To achieve "bulletproof" stability, we use a hybrid memory strategy:

1. **Static DRAM (Guaranteed Stability)**:
   - Core management objects (e.g., `DisplayManager`, `ConfigManager`) and board slots are allocated **statically**.
   - This ensures that if the device boots, the core features have guaranteed RAM that can never be "lost" to fragmentation.

2. **RAII Heap (Transient Flexibility)**:
   - Large or unpredictable objects (e.g., `WiFiClientSecure`, `JsonDocument`, `HTTPClient`) are heap-allocated using **Smart Pointers** (`std::unique_ptr`).
   - They are created only when needed (e.g., during a data fetch or MCP call) and destroyed immediately after, freeing up the "headroom" for the next task.

### Flash-First (PROGMEM)
- All static text, JSON schemas, and web portal assets are stored in **Flash memory** (PROGMEM) rather than RAM.
- We use the `FPSTR()` and `F()` macros to ensure these strings are spooled directly from flash to the output stream.

### Streaming JSON Performance
Instead of building a massive `JsonDocument` in RAM (which would crash the 8KB task stack), the MCP server uses **Incremental Streaming**:
```cpp
// Example: Direct streaming to avoid heap spikes
outputStream.print("{\"jsonrpc\":\"2.0\",\"result\":{\"content\":[{\"type\":\"text\",\"text\":\"");
printChunkedBase64(rawFramebuffer, len, outputStream);
outputStream.print("\"}],\"isError\":false}}");
```

---

## 4. User Manual: Interacting with the Board

While primarily used by AI agents, developers can interact with the server using standard HTTP tools like `curl`.

### Listing Available Tools
**Request:**
```bash
curl -X POST http://[DEVICE_IP]/mcp -d '{"jsonrpc":"2.0","method":"tools/list","id":1}'
```

### Common Tools
- `get_system_telemetry`: Returns free heap, uptime, and internal temperature.
- `get_display_buffer`: Returns the raw framebuffer as a Base64-encoded string.
- `get_active_data`: Returns a full JSON dump of currently cached transit and weather data.
- `list_files` / `get_file_raw`: Internal storage inspection for configuration audits.

---

## 5. Agent Skill: `device-mcp-client`

The `.agents/skills/device-mcp-client` skill is the "driver" that allows the Antigravity agent to communicate with your board.

### Automation Scripts
- **`discovery.py`**: Automatically finds the board's IP address from your serial logs.
- **`diagnostics.py`**: Generates a health report.
- **`capture_display.py`** / **`captureU8g2.py`**: These scripts perform the heavy lifting of pulling the Base64 frame, decoding it, and generating a `PNG` in your project workspace.

> [!TIP]
> If you are debugging a layout alignment issue, you can simply ask the agent to **"capture a screenshot of the display"**. The agent will use this skill to "see" exactly what is rendered on your physical OLED panel.

---

## Reference Documents
* [System Specification Document](SystemSpecificationDocument.md)
* [Memory Management Reference](reference/MemoryArchitecture.md)
* [Device-Side MCP Specification](reference/DeviceSideMCPServerSpecification.md)
