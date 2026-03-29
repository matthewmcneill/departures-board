# Migration & Upgrade Testing Strategy

Your proposed approach is a very common starting point, but we can significantly optimize it. Testing firmware upgrades on embedded devices (especially ESP32) can be painfully slow if handled purely manually. 

Below is an assessment of your original approach followed by an optimized, repeatable strategy that leverages PlatformIO's native tooling.

## 1. Critique of the Original Approach

> *Flash original repo -> Configure manually -> Pull off configs -> Flash new build with old config -> Verify*

While this works, it comes with several architectural and workflow friction points:

*   **Compilation Overhead**: Checking out the old repository requires downloading old dependencies and running a full `pio run`, which can be slow and prone to environment regressions (e.g., old code failing to compile on newer compilers).
*   **Manual Configuration**: Typing configurations into the Web UI for every test cycle is unrepeatable, brittle, and blocks CI automation.
*   **Complex Extraction**: "Pulling off" the configuration from an ESP32 `LittleFS` partition requires precise `esptool.py read_flash` commands to slice the exact partition memory addresses, followed by reverse-mounting the binary image on your PC. It's difficult to script simply.
*   **Hardware Bottleneck**: The entire workflow ties up the physical hardware lock for 10-15 minutes per cycle.

---

## 2. The Improved Approach: "Synthetic Data Injection"

Because the `ConfigManager` handles the data schemas predictably, **we do not need the old firmware to generate old data.** We only need the *shape* of the old JSON. We can use PlatformIO's built-in file system tools to inject this directly.

### Phase 1: Mocking Legacy Schemas (Once)
Instead of extracting data from a device, we hand-craft static JSON files that represent old firmware states. (I can see in `configManager.cpp` exactly how the `< v2.0` `apikeys.json` and `< v2.3` `config.json` look).
We store these in your repo (e.g., `test/fixtures/legacy_v1_apikeys.json`).

### Phase 2: PlatformIO File System Upload 
PlatformIO has a dedicated tool chain for LittleFS. When you are ready to test a migration:
1. Create a `data/` directory in the root of your project.
2. Copy your mocked `config.json` and `apikeys.json` into this `data/` directory.
3. Run **`pio run -t uploadfs`**.
   * *This compiles the files into a LittleFS binary image and flashes it cleanly onto the ESP32's storage partition, bypassing the firmware entirely.*

### Phase 3: Flash and Boot the Target Firmware
1. Flash your latest, bleeding-edge firmware (`pio run -t upload`).
2. Boot the device while monitoring the Serial output (`pio device monitor`).
3. **The Migration triggers locally**: `ConfigManager::loadConfig()` will naturally read the old JSON, realize the versions are `1.0`, execute its native migration blocks, and actively re-save the files in the new hierarchical format.

### Phase 4: Automated Verification
Instead of clicking around the UI to see if it worked, you can programmatically verify:
1. Ping the new device's `/api/config` HTTP endpoint and assert that the legacy fields successfully translated to their new homes.
2. Look for the `LOG_INFO("CONFIG", "Performing v2.2 multi-board migration...");` strings in the Serial console.

---

## 3. Why this is the Better Approach

> [!TIP]
> **Speed & Automation**
> This process takes seconds instead of minutes. It eliminates the need to compile legacy branch code, making the testing cycle purely focused on the *current* code's ability to migrate schemas.

> [!IMPORTANT]
> **Extensibility**
> By relying on static JSON files rather than manual UI configuration, you can easily create edge-case payloads. Want to test what happens if a user's legacy config was corrupted? Just create a `corrupted_legacy.json` mock and run `uploadfs`.

### Suggested Next Steps:
If you want to adopt this approach, we can:
1. Create a `test/fixtures/` directory containing the known legacy `config.json` / `apikeys.json` snapshots.
2. Write a lightweight helper script (e.g., `scripts/flash_legacy_state.py`) that copies a specific fixture into the `data/` folder and triggers the `uploadfs` target for you automatically.
