---
name: device-mcp-client
description: Interacts with the ESP32 Device MCP Server through JSON-RPC to query telemetry, configure parameters, and capture real-time display buffers to image files.
---

# `device-mcp-client`

This skill provides native utilities to interact with a flashed ESP32 running the JSON-RPC Device MCP server via HTTP.

## Overview
Because the agent cannot visually "see" the physical ESP32, this skill provides a suite of Python scripts that bridge the gap. It automatically discovers the dynamic IP of the board from hardware logs, safely pings its telemetry endpoints, and can download and decode the raw RGB565 framebuffer into a visual `PNG` artifact for debugging UI layouts.

## Scripts

### `scripts/discovery.py`
Parses the active `pio-manager` spool cache to locate the dynamically assigned DHCP IP address from the ESP32's boot sequence.
**Usage:** `python3 .agents/skills/device-mcp-client/scripts/discovery.py`

### `scripts/capture_display.py`
Fetches the `get_display_buffer` frame from the target IP, decodes the Base64 RGB565 binary, and converts it to a standard PNG. Use this for color displays.
**Usage:** `python3 .agents/skills/device-mcp-client/scripts/capture_display.py [IP_ADDRESS] [OUTPUT_PATH]`

### `scripts/captureU8g2.py`
Fetches the `get_display_buffer` frame and decodes the U8G2 monochrome tile-based buffer. **Use this for the current Departures Board hardware revision (SSD1322).**
**Usage:** `python3 .agents/skills/device-mcp-client/scripts/captureU8g2.py [IP_ADDRESS] [OUTPUT_PATH]`

### `scripts/diagnostics.py`
Queries the device's current memory, temperature, uptime, and network stats by invoking the `get_system_telemetry` and `get_network_status` MCP tools.
**Usage:** `python3 .agents/skills/device-mcp-client/scripts/diagnostics.py [IP_ADDRESS]`
