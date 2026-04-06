# Comprehensive Secure OTA & Release Implementation Plan

This document outlines the architectural changes required to implement a cryptographically secure, fully managed release pipeline, including your requests for quiet-hour polling, passive update checks, and partitioning rollback.

## Proposed Changes

### Phase 1: Cryptographic Tooling & Scripts

#### [NEW] `scripts/generate_keys.sh`
- A Bash script using OpenSSL to generate an AES-256 password-protected `private_key.pem`.
- Extracts the public key and formats it into a C++ string, outputting it automatically to `src/publicKey.hpp`.

#### [NEW] `scripts/deploy_release.py`
- Python script that reads the local firmware version and queries the public GitHub Repository to verify it is strictly incremental.
- Triggers PlatformIO to compile the `firmware.bin` payload.
- Prompts the user for the AES-256 password to hash and sign the `.bin` using `private_key.pem`, yielding `firmware.sig`.
- Interfaces with the GitHub REST API (or `gh` CLI) to securely publish the Release and upload both assets.

### Phase 2: C++ Firmware Security (mbedtls)

#### [MODIFY] `lib/HTTPUpdateGitHub/HTTPUpdateGitHub.cpp`
- **Signature Downloader:** Re-architect `handleUpdate` to locate and download the `firmware.sig` (256 bytes) directly into RAM. If missing, 404, or malformed, the process cleanly aborts before wasting bandwidth on the primary `.bin` file.
- **Boot Validation Hook:** Implement a streaming `mbedtls_sha256_update()` block during the `.bin` download flow.
- Ensure `esp_ota_set_boot_partition()` is *only* called if `mbedtls_pk_verify()` evaluates the hashed stream successfully against `src/publicKey.hpp`. Otherwise, the update is safely discarded.

### Phase 3: Configuration & Polling Rules

#### [MODIFY] `modules/configManager/configManager.cpp` & `.hpp`
- Add `int otaQuietHour = 3;` (default 3 AM) to the `Config` struct.
- Wire JSON serialization (`updateQuietHour`) in `save()` and `loadConfig()`.

#### [MODIFY] `lib/otaUpdater/otaUpdater.cpp` & `.hpp`
- Change `tick()` to execute its automated daily update check only when `timeinfo.tm_hour == cfg.otaQuietHour`.
- **`checkUpdateAvailable(String& outVersion)`**: Abstract the GitHub `ghUpdate` polling logic to gracefully return `true/false` without commandeering the display matrix.
- **`forceUpdateNow()`**: Provide an explicit public method to ignore passive version delays and initiate the secure validation download.
- **`rollbackFirmware()`**: Implement a public method that queries `esp_ota_get_next_update_partition(NULL)` to find the inactive backup partition, explicitly calls `esp_ota_set_boot_partition()`, and triggers an `ESP.restart()`.

### Phase 4: Web API Exposure

#### [MODIFY] `modules/webServer/webHandlerManager.cpp`
- `/api/ota/available` (GET): Returns JSON payload confirming update status (`{"available": true, "version": "v1.2.3"}`).
- `/api/ota/force` (POST): Wires to `otaUpdater.forceUpdateNow()`.
- `/api/ota/rollback` (POST): Wires to `otaUpdater.rollbackFirmware()`.
- Ensure config endpoints accommodate the new `otaQuietHour` property for subsequent Web UI layout development.

## User Review Required

> [!CAUTION]
> The cryptographic implementation relies on software-level `mbedtls` evaluation in `HTTPUpdateGitHub.cpp`, meaning your hardware eFuses remain completely unburnt and non-destructive. If you ever lose your private key password, you can easily plug the board back in via USB to flash a new `publicKey.hpp`. It is highly secure, but retains a recovery route.

> [!NOTE]
> As instructed, this phase focuses entirely on Script generation and C++ Backend endpoints. HTML/JavaScript logic for the interactive UI buttons is out of scope for this plan unless you specify otherwise.

## Verification Plan

### Automated Tests
- Test Python deployment script against a mock GitHub repository/tag to verify version rejection logic.
- Test OpenSSL signature output against `mbedtls` C-code simulator to guarantee identical padding schemes. 

### Manual Verification
- Deploy firmware v1.0. 
- Run the python deployment script for v1.1.
- Send a `POST /api/ota/force`. Firmware should securely stream, validate the signature, and reboot natively to v1.1.
- Send a `POST /api/ota/rollback`. Firmware should point the flag back to the active sector, reboot, and boot normally into v1.0.
