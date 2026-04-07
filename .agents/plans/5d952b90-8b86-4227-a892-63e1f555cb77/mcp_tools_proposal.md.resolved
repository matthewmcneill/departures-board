# Proposed Device-Side MCP Tools

Based on the `DeviceSideMCPServerSpecification.md` and the existing `departures-board` architecture, here is a comprehensive list of MCP services that could be exposed from the ESP32. 

Because we have a strict ~250KB Flash budget, tools should have concise PROGMEM schemas. 

## 1. System State & Telemetry (`appContext`, `timeManager`, `logger`)
* **`get_system_telemetry`** *(Your point A)*: 
  * Returns: Uptime, free heap, minimum free heap (critical for memory leak detection), current CPU frequency, and NTP synchronization status/local time.
* **`get_recent_logs`**: 
  * If `logger` implements an in-memory ring buffer, the AI agent can remotely pull logs without needing a USB serial connection.
* **`reboot_device`**: 
  * Safely trigger an `esp_restart()`, useful for clearing transient memory states.

## 2. Configuration Management (`configManager`)
* **`get_configuration`** *(Your point B)*: 
  * Streams the active, fully serialized `config.json` state.
* **`patch_configuration`**: 
  * Allows the AI agent to update specific nested JSON keys dynamically and save them to flash without needing the user's web portal.

## 3. Data Sources & Content (`dataManager`, `schedulerManager`)
* **`get_active_data`** *(Your point C)*: 
  * Interrogate `dataManager` to return the currently parsed domain data (TfL status, National Rail departures, RSS news, Weather). 
* **`force_data_refresh`**: 
  * Command the `schedulerManager` to instantly trigger an API poll across configured sources.
* **`inject_mock_data`** *(Highly Recommended)*: 
  * Temporarily overwrite the internal state of a data source with spoofed JSON from the AI agent. This allows the AI to force the board to display edge cases (like a "Delayed 150 mins" train) to verify the UI layout without waiting for reality to match the scenario.

## 4. Display Control (`displayManager`)
* **`get_display_buffer`** *(Your point D)*: 
  * Return the current graphical Matrix/Neopixel buffer. *Implementation note: Due to size, the bytearray should be Base64 encoded or Run-Length Encoded to avoid huge JSON arrays that might exhaust memory buffers.*
* **`set_display_brightness`**: 
  * Dynamically scale the global PWM/brightness setting (0-255).
* **`render_debug_message`**: 
  * Take temporary graphical control of the display to print arbitrary text to visually verify AI connectivity.

## 5. Network & Updates (`wifiManager`, `otaUpdater`)
* **`get_network_status`**: 
  * Yield IP address, subnet, MAC address, and Wi-Fi signal strength (RSSI). Dropping RSSI could explain sudden API timeout bugs.
* **`trigger_ota_check`**: 
  * Command the firmware to hit GitHub immediately and check for/apply an update.

## 6. Hardware Interactions (`buttonHandler`)
* **`simulate_button_event`**: 
  * Inject a synthetic event (e.g., "short_press") into `buttonHandler`. If the physical button cycles data pages, the agent can remotely cycle the UI state.

---

### Implementation Recommendation
Due to the **250KB Flash / low RAM limits**, I recommend starting with an MVP Core Quartet:
1. `get_system_telemetry`
2. `get_configuration`
3. `reboot_device`
4. `get_display_buffer`

This provides baseline observability, configuration state, visual verification, and an "escape hatch" (reboot), establishing an incredibly powerful agent-to-device loop with minimal code overhead.
