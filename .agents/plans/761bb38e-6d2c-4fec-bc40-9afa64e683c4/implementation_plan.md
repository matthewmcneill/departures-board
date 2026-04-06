# Implementation Plan: Epoch Translation & Version-Locked Suffixing

This document outlines the concrete strategy for establishing the dual-architecture configuration pipeline:
1. **Strategy A (Translation):** A standalone translation layer to convert unversioned Gadec schemas into modern nested structures in RAM.
2. **Strategy B (Storage):** A non-destructive disk strategy to save upgraded permutations as distinct suffix versions (e.g., `config_2_6.json`), insulating the user's base `config.json` against destructive overwriting so they can safely revert firmware.

## Proposed Changes

---

### configManager Module

#### [NEW] [gadecMigration.hpp](file:///Users/mattmcneill/Personal/Projects/departures-board/modules/configManager/gadecMigration.hpp)
A new standalone utility namespace dedicated strictly to sniffing and translating unversioned or legacy configurations into the latest state before `configManager` processes them. 

```cpp
#pragma once
#include <ArduinoJson.h>

namespace GadecMigration {
    enum UpstreamEpoch {
        EPOCH_LATEST_NATIVE = 0,
        EPOCH_GADEC_V1 = 1,
        EPOCH_GADEC_V2 = 2,
        EPOCH_UNKNOWN = 99
    };

    UpstreamEpoch detectConfigEpoch(JsonObject root);
    bool translateToModern(JsonDocument& doc, UpstreamEpoch epoch);
}
```

#### [NEW] [gadecMigration.cpp](file:///Users/mattmcneill/Personal/Projects/departures-board/modules/configManager/gadecMigration.cpp)
Extracts the legacy conversions originally bloating `configManager.cpp`. It forcefully projects flat variables (`tubeLat`, `busFilter`, `sleepStarts`) into the modern `boards` array, `feeds` object, and `schedules` array. It implicitly forces `"version": 2.6` onto the document state.

#### [MODIFY] [configManager.hpp](file:///Users/mattmcneill/Personal/Projects/departures-board/modules/configManager/configManager.hpp)
- Introduce constants for the active configuration format natively to prevent arbitrary hardcoding:
  ```cpp
  #define CONFIG_VERSION_MAJOR 2
  #define CONFIG_VERSION_MINOR 6
  ```
- Declare a static or member utility (e.g., `String getActiveConfigFilename()`) that uses these macros to dynamically construct and return the required file path (`"/config_2_6.json"`), replacing raw string dependency.

#### [MODIFY] [configManager.cpp](file:///Users/mattmcneill/Personal/Projects/departures-board/modules/configManager/configManager.cpp)
**1. `loadConfig()` Refactoring:**
- Introduce Version Hunting backwards:
  1. Check for `getActiveConfigFilename()` (e.g., `/config_2_6.json`).
  2. If missing, enumerate `LittleFS.openDir("/")` looking for the most recent `config_X.json` fallback.
  3. If missing, fallback to parsing the standard upstream `/config.json`.
- After reading the payload, invoke `GadecMigration::detectConfigEpoch()`.
- If the epoch is not `EPOCH_LATEST_NATIVE`, invoke `translateToModern()`.
- Strip out the massive inline migration code block (lines `~588` to `~745`).

**2. `save()` & `writeDefaultConfig()`:**
- Redirect both write operations to strictly target `getActiveConfigFilename()`. `config.json` is never updated or written to, ensuring safe downstream reversals.

## Open Questions

- **Pruning Strategy:** To prevent LittleFS disk exhaustion across multiple future OTA updates, should we append a garbage-collection hook in `save()` that deletes any configuration file that is *older* than `getActiveConfigFilename()` **except** for `config.json`?
- **The API Key Desync:** Currently, when we detect `apikeys.json`, we migrate it securely to `apikeys.bin` and then explicitly delete `apikeys.json`. If a user reverts to Gadec firmness, it will technically be broken until they re-input their API keys because we destroyed the plaintext file. Is this acceptable in the name of security, or should we intentionally preserve `apikeys.json` to allow 100% flawless reversing?
