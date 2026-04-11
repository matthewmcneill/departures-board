# Plan Audit Status
- [x] House Style Documentation Checked
- [x] Architectural Standards Checked
- [x] Embedded Systems Impact Assessment Added

# Migrate Feeds Configuration into Nested Schema

This plan aligns the internal ESP32 `config.json` structure with the modern Web Portal schema by grouping the RSS and Weather Key configurations into a dedicated `feeds` JSON object. Currently, these persist flatly in the root of the file on flash.

## Goal Description
Resolve duplication of rssUrl/rssName in the config json structure and natively structure them under `feeds` on the device's flash storage, aligning standard web-UI bindings.

## User Review Required

> [!IMPORTANT]  
> Bumping the `configVersion` to `2.6` means devices upgrading will perform a one-time save upon first boot to migrate from the legacy flat `rssUrl` variables to the novel `feeds.rss` layout on disk.

## Resource Impact Assessment
- **Flash/ROM**: Marginal increase (few bytes) due to slightly longer string literals (`"feeds"` object key) representing the JSON scope.
- **RAM / Heap**: Nested objects in ArduinoJson `JsonDocument` require slightly more heap space (1-2 extra blocks) during serialization/deserialization. Given that `configManager` only operates during initialization or user API commits, max allocated blocks might spike momentarily but well within safe operating limits.
- **Power**: Negligible impact. Only a one-time extra flash write occurs during the migration boot cycle.
- **Security**: No change. Weather tokens remain secure and handled internally.

## Proposed Changes

### `configManager.cpp`

1. **Update `ConfigManager::save()`** to serialize RSS and Weather config into a nested `"feeds"` object instead of the root.
2. **Update `ConfigManager::loadConfig()`**:
   - Add backwards-compatible reading logic so that it first checks for the modern nested `"feeds"` object block before falling back to the legacy flat schema (`rssUrl`, `rssName`, `weatherKeyId`).
   - Add a migration block for `(loadedVersion < 2.6f)`. If triggered, it logs a migration message, updates `config.configVersion = 2.6f`, and performs an immediate `save()` to re-write the JSON file securely on flash with the new nested schema.

#### [MODIFY] [configManager.cpp](modules/configManager/configManager.cpp)

**Save Changes:**
```cpp
// Remove the existing flat values:
// doc[F("rssUrl")] = config.rssUrl;
// doc[F("rssName")] = config.rssName;
// doc[F("weatherKeyId")] = config.weatherKeyId;

// Add modern nested formatting:
JsonObject feeds = doc[F("feeds")].to<JsonObject>();
feeds[F("rss")] = config.rssUrl;
feeds[F("rssName")] = config.rssName;
feeds[F("weatherKeyId")] = config.weatherKeyId;
```

**Load Changes:**
```cpp
// Legacy Check (Fallback)
if (settings[F("rssUrl")].is<const char *>())
    strlcpy(config.rssUrl, settings[F("rssUrl")], sizeof(config.rssUrl));
if (settings[F("rssName")].is<const char *>())
    strlcpy(config.rssName, settings[F("rssName")], sizeof(config.rssName));
if (settings[F("weatherKeyId")].is<const char *>())
    strlcpy(config.weatherKeyId, settings[F("weatherKeyId")], sizeof(config.weatherKeyId));

// Modern Nested Schema (Overrides legacy if it exists)
if (settings[F("feeds")].is<JsonObject>()) {
    JsonObject f = settings[F("feeds")];
    if (f[F("rss")].is<const char *>())
        strlcpy(config.rssUrl, f[F("rss")], sizeof(config.rssUrl));
    if (f[F("rssName")].is<const char *>())
        strlcpy(config.rssName, f[F("rssName")], sizeof(config.rssName));
    if (f[F("weatherKeyId")].is<const char *>())
        strlcpy(config.weatherKeyId, f[F("weatherKeyId")], sizeof(config.weatherKeyId));
}

config.rssEnabled = (config.rssUrl[0] != '\0');
```

**Migration Block:**
```cpp
if (loadedVersion < 2.6f) {
    LOG_INFO("CONFIG", "Performing v2.6 migration (moving feeds to nested object)...");
    config.configVersion = 2.6f;
    save();
}
```

### `webHandlerManager.cpp`

1. **Cleanup `/api/config` Export mappings**: The current export mapping explicitly replicates legacy formats: `system["rssUrl"] = config.rssUrl`. I will remove the legacy duplicates being pushed into the `system` block, keeping only the modern `feeds` block mapping.

#### [MODIFY] [webHandlerManager.cpp](modules/webServer/webHandlerManager.cpp)

- In `handleGetConfig`, remove `system["rssUrl"]` and `system["rssName"]` mapping. Keep the existing `feeds` mappings. 
- Ensure `handleSaveAll` handles `feeds` (it already does, but we should make sure it doesn't accidentally save into legacy `config.rss` from the system `sys` block).

## Verification Plan

### Automated Tests
- Run `npm run test` in `/test/web/` to confirm that removal of duplicated `rssUrl` properties from the `/api/config` payload doesn't break the frontend's ability to render or save feeds.

### Manual Verification
- Simulate a legacy configuration load (creating a sample config with flat feeds).
- Monitor the mock environment logs to observe the v2.6 migration message and confirm the rewritten file matches the nested schema.
