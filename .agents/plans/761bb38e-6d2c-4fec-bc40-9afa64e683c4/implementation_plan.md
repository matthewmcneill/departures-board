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

## Open Questions & Resolved Decisions

- **Resolved: API Key Desync**: When `apikeys.bin` is successfully written, `apikeys.json` will be deleted for security. Reverting to older firmware will require re-entering keys.
- **Resolved: Pruning Strategy**: To prevent LittleFS exhaustion, `configManager::save()` will implement a "Keep 3" rule:
    1. **Always Keep**: `config.json` (Gadec baseline).
    2. **Rotate**: Keep the 3 most recent versioned files (e.g., `config_2_6.json`, `config_2_5.json`, `config_2_4.json`) and delete older versions during the save operation.

## Verification Plan

### Automated Tests
- Mock LittleFS with multiple versioned files and verify `huntForLatestConfig()` logic.
- Verify `GadecMigration` translations via unit tests with legacy JSON payloads.

### Manual Verification
1. Flash firmware, verify successful migration of `config.json` to `config_2_6.json`.
2. Verify `config.json` remains intact on disk.
3. Verify `apikeys.json` is removed after `apikeys.bin` creation.
