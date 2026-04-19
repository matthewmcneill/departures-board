# Documentation Wizard Auto-Capture

## Goal Description
The objective is to overhaul `docs/ConfiguringDevice.md` by capturing a complete, step-by-step walkthrough of the ESP32 configuration process. Instead of taking physical photos of the OLED screen, we will utilize the `.agents/skills/device-mcp-client` tools to discover the device, monitor telemetry, and capture pixel-perfect digital screenshots of the hardware display. Concurrently, a Browser Subagent will navigate the web UI and record WebP videos of the user-facing configuration portal.

## User Review Required
> [!IMPORTANT]
> The hardware backend for screenshots (JSON-RPC) has been successfully deployed. This revised plan relies purely on orchestrating Python scripts (`discovery.py`, `captureU8g2.py`, `diagnostics.py`) and a Browser Subagent to generate the documentation assets. Please review the workflow below and approve via `/plan-start`.

## Phase 3: Hybrid Build Finalization (Linker & Cleanup)

The ESP-IDF aggressively parses `managed_components` and expects `app_main` to be defined unless told otherwise. We will implement the correct compiler mappings for the hybrid build without hacky workarounds.

### User Review Required
Removing `managed_components` guarantees your binary maintains zero bloat from unrequested ESP Insights / RainMaker connectivity paths. 

### Configurations
#### [MODIFY] [sdkconfig.defaults](file:///Users/mcneillm/Documents/Projects/departures-board/sdkconfig.defaults)
- Add `CONFIG_AUTOSTART_ARDUINO=y` (Tells ESP-IDF to automatically map C++ `app_main` to the Arduino `setup/loop` core)
- Add `CONFIG_ESP_INSIGHTS_ENABLED=n` (Optional defensive measure against RainMaker)

#### [MODIFY] [platformio.ini](file:///Users/mcneillm/Documents/Projects/departures-board/platformio.ini)
- Remove the `board_build.embed_txtfiles` certificates hack.
- Add `NetworkClientSecure` to `lib_deps` explicitly to prevent the `_Z16start_ssl_client` unlinked target error.

#### [DELETE] `managed_components/` and `dependencies.lock`
- Erase the cached ESP-IDF auto-fetched components directory to prevent the Certificate assembly bug gracefully.

## Open Questions
- Do you approve of wiping the `managed_components` dir to remove the RainMaker bloating, and applying the `CONFIG_AUTOSTART_ARDUINO` configuration?

## Verification Plan
1. Delete ESP-IDF cache.
2. Run `~/.platformio/penv/bin/pio run -t clean`
3. Run `~/.platformio/penv/bin/pio run -e esp32s3nano`
> [!WARNING]
> **Censoring Sensitive Data:** We need to explicitly censor WiFi SSIDs, passwords, and API tokens during the Subagent's web navigation. I propose instructing the Browser Subagent to execute DOM manipulation (e.g. replacing text inputs with asterisks `***`) *before* finalizing the video captures. Are you comfortable with this approach? 

## Proposed Changes

We will orchestrate the documentation generation primarily through commands rather than source code changes.

### 1. Device Discovery & Monitoring
- **Discovery**: Execute `python3 .agents/skills/device-mcp-client/scripts/discovery.py` to auto-detect the dynamically assigned IP from the `pio-manager` spooling cache.
- **Diagnostics**: Run `python3 .agents/skills/device-mcp-client/scripts/diagnostics.py [IP]` to ensure the device has booted successfully and has enough free heap for capturing.

### 2. Browser Subagent Integration
- Instantiate a **Browser Subagent** targeting the ESP32's web portal IP address.
- The subagent will click through the `WiFiManager` startup and the Main Portal settings.
- Recordings will be captured and output as high framerate WebP videos for documentation.

### 3. Hardware Display Capture
- At each key step (e.g., AP Mode, Connection Success, API Setup), we will execute `python3 .agents/skills/device-mcp-client/scripts/captureU8g2.py [IP] assets/oled_screen_N.png`.
- This ensures pixel-perfect monochrome representation of the physical 256x64 SSD1322 OLED display.

### 4. [MODIFY] `docs/ConfiguringDevice.md`
- Rewrite the documentation narrative to incorporate the newly captured `webp` videos and OLED `png` screens.
- Replace the legacy descriptions with the updated, highly visual "First Time Configuration" and "General Settings" walkthroughs.

## Verification Plan
### Automated Tests
- Validate `discovery.py` and `diagnostics.py` successfully connect and return valid JSON output without timing out.
- Verify `captureU8g2.py` successfully dumps and saves a valid 256x64 PNG artifact without exceptions.

### Manual Verification
- Visually review the finalized `docs/ConfiguringDevice.md`.
- Confirm sensitive API tokens and WiFi passwords are obscured in the generated WebP recordings and images.
