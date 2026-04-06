# Goal Description
The core issue fundamentally stems from the build pipeline. The `web/index.html` file changes were never actually making it onto the ESP32 Nano hardware because the `portalBuilder.py` Python script (which compresses and bundles the portal HTML/CSS into the C++ `portalAssets.h` header) was accidentally missing from the `[env:esp32s3nano]` build configuration in `platformio.ini`. We were modifying the HTML file but uploading the old cached binary blob every time.

## User Review Required
No major architectural impact. This is purely rectifying a build script omission for the new Nano hardware definition.

## Proposed Changes

### Configuration
#### [MODIFY] [platformio.ini](file:///Users/mcneillm/Documents/Projects/departures-board/platformio.ini)
- Add `pre:scripts/auto_build_wasm.py` to the `extra_scripts` section of `[env:esp32s3nano]`.
- Add `pre:scripts/portalBuilder.py` to the `extra_scripts` section of `[env:esp32s3nano]`.

#### [MODIFY] [configManager.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/configManager/configManager.cpp)
- Remove the injection of the 4 dummy display boards (National Rail, TfL, Bus, etc.) from `ConfigManager::writeDefaultConfig()`. This allows a pristine device to correctly enter the `BOARD_SETUP` system state and display the `SystemBoardId::SYS_SETUP_HELP` wizard on the hardware OLED, rather than awkwardly trying to rotate through 4 unconfigured layouts.

### Web Portal Frontend
#### [MODIFY] [index.html](file:///Users/mcneillm/Documents/Projects/departures-board/web/index.html)
- Revert the CSS `display: none`/`display: flex` overwrite I added earlier.
- Restore the original "hack" snippet so it accurately tests the intended behaviour: `body.ap-mode .tab-link:not([data-target="tab-wifi"]):not([data-target="tab-system"]) { display: none; }` 

## Open Questions
None.

## Verification Plan
1. Claim hardware lock.
2. Manually trigger `./.agents/skills/hardware-testing/scripts/safe-flash.sh`.
3. Check the output logs to verify that `portalBuilder.py` triggers successfully prior to compilation, embedding the new UI.
4. User validates the "System" tab reappears in Chrome/iOS in real life tests in AP mode.
