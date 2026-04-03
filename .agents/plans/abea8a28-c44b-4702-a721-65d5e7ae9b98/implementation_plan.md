# NVS Security Migration (WiFi & API Keys)

[x] Reviewed by `house-style-documentation` - passed (Goal, Review Required, Proposed Changes, Verification Plan present, and repository-relative file paths used).
[x] Reviewed by `architectural-refactoring` - passed (DeviceCrypto will be instantiated inside appContext as an infrastructure library for Dependency Injection to adhere to encapsulation standards).
[x] Reviewed by `embedded-web-designer` - N/A (No Web UI modifications).
[x] Reviewed by `embedded-systems` - passed (Resource Impact updated with mbedtls evaluation).

## Goal Description

The objective of this plan is to deprecate vulnerable plaintext LittleFS storage for `wifi.json` and `apikeys.json`. By migrating to the ESP-NVS partition via `Preferences.h`, this plan ensures that sensitive WiFi credentials and API tokens are securely isolated from standard non-sensitive filesystem layouts. 

## User Review Required

> [!IMPORTANT]
> Please review the strategy for migrating existing `wifi.json` and `apikeys.json` blobs to `Preferences.h`. Is it acceptable to use JSON serialization for the API keys within NVS to preserve existing registry structures, or would you prefer a flat binary `putBytes()` struct implementation for the API keys?

## Resource Impact Assessment

- **Memory (RAM/Heap/Stack)**: `mbedtls` cryptographic operations (AES-256-CBC) will require dynamic allocation of context structs during `loadWiFiConfig()` and `saveWiFiConfig()`. This buffer allocation is transient and will be strictly scoped and freed. The 32-byte master AES key will reside permanently in RAM within the `DeviceCrypto` instance.
- **Flash/ROM**: Including `mbedtls` routines adds a minor footprint to the compiled binary size (typically 4-8 KB).
- **PSRAM**: Unaffected.
- **Power**: Unaffected.
- **Security**: **Critical Improvement**. Storing secrets in encrypted form on LittleFS, bound by a unique device key stored in the hardware NVS partition, mitigates local plaintext extraction. This ensures that even if the filesystem is dumped (via standard web server vulnerabilities or serial dump), the sensitive WiFi and API tokens are unreadable without the NVS-bound key.

## Proposed Changes

### [NEW] [deviceCrypto.hpp/cpp](lib/deviceCrypto/deviceCrypto.hpp)
To maintain OTA compatibility while securing data, we will implement a hardware-bound encryption layer adhering to the project's Dependency Injection pattern.

- **NVS-Bound Key**: We will store a unique 256-bit AES key in the existing NVS namespace `"secrets"`. 
- **Encryption**: We will use `mbedtls` (AES-256-CBC) to encrypt/decrypt sensitive string payloads.
- **Key Generation**: On the first boot of the new firmware, if no key exists in NVS, the device will generate a unique key using the ESP32 hardware True Random Number Generator (`esp_random()`).

### `modules/appContext`
In adherence to the Architectural Standards (`appContext` acts as the System Hub), the `DeviceCrypto` service will be instantiated here and passed by reference to other managers.

#### [MODIFY] [appContext.hpp](modules/appContext/appContext.hpp)
- Include `deviceCrypto.hpp`.
- Add `DeviceCrypto deviceCrypto;` to the private class attributes to control its lifecycle.
- Add `DeviceCrypto& getDeviceCrypto() { return deviceCrypto; }` accessor.

#### [MODIFY] [appContext.cpp](modules/appContext/appContext.cpp)
- Call `deviceCrypto.begin()` during the boot sequence BEFORE `configManager.loadApiKeys()` and `wifiManager.begin()`, ensuring the key is mounted and ready for dependent cryptographic functions.

### `modules/wifiManager`
Migrate `wifiManager.cpp` to use the `DeviceCrypto` for credential loading and saving.

#### [MODIFY] [wifiManager.cpp](modules/wifiManager/wifiManager.cpp)
- **`loadWiFiConfig()`**:
  - Check if `/wifi.json` exists (Legacy plaintext). 
  - If yes, load, encrypt via `DeviceCrypto`, save to `/wifi.bin`, and delete `/wifi.json`.
  - Otherwise, load and decrypt from `/wifi.bin`.

- **`saveWiFiConfig()`**: Encrypt credentials and save to `/wifi.bin`.
- **`resetSettings()`**: Remove `/wifi.bin` and `/wifi.json`.

---

### `modules/configManager`
Migrate API key serialization to use `DeviceCrypto` for encrypted storage in LittleFS.

#### [MODIFY] [configManager.cpp](modules/configManager/configManager.cpp)
- **`saveApiKeys()`**:
  - Encrypt the JSON registry string using `DeviceCrypto`.
  - Save the resulting binary blob to `/apikeys.bin`.
- **`loadApiKeys()`**:
  - Check if `/apikeys.bin` exists.
  - If missing, check for legacy `/apikeys.json`.
  - If legacy found: Load, migrate, encrypt, save to `/apikeys.bin`, and delete `/apikeys.json`.
  - Otherwise, decrypt from `/apikeys.bin`.

## Verification Plan

### Automated Tests
- Build and compilation validation utilizing PlatformIO (`pio run`).
- Verify `mbedtls` includes correctly resolve.

### Manual Verification
- **OTA Success**: Deploy via OTA and verify the system automatically migrates existing JSON files to encrypted BIN files without requiring a USB flash.
- **Factory Reset**: Perform a factory reset and verify the NVS encryption key is regenerated (or cleared) and new credentials work.
- Connect via serial terminal and confirm `/wifi.json` and `/apikeys.json` are NOT logged as present in LittleFS.
- Trigger a reboot and ensure the API keys and WiFi credentials load successfully from NVS.
