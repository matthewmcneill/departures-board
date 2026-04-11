# Resolve Library Deprecation Warnings (v3.1 Cleanup)

## Audit Checklist
- [x] **House Style**: camelCase file naming and standard headers confirmed.
- [x] **Architectural Standards**: Maintaining backward compatibility with legacy configs via modern syntax.
- [x] **Resource Impact**: Negligible impact on memory/power.

## User Review Required

> [!NOTE]
> These changes are purely syntactic and do not alter the logic of the configuration migration or web delivery. No functional regression is expected.

## Proposed Changes

### 🌐 webServer

#### [MODIFY] [webHandlerManager.cpp](modules/webServer/webHandlerManager.cpp)
- Replace `beginResponse_P` with `beginResponse`. The modern API automatically handles Flash-resident buffers without the `_P` suffix.

### ⚙️ configManager

#### [MODIFY] [gadecMigration.cpp](modules/configManager/gadecMigration.cpp)
- Replace legacy `.containsKey()` calls with modern, type-safe `.is<T>()` or `.isNull()` checks.
- **Line 25**: `root.containsKey("version")` -> `root["version"].is<float>()`
- **Line 34**: Check `crs`, `tubeId`, `busId` presence using `!root["key"].isNull()`.
- **Line 39 & 57**: `root.containsKey("boards")` -> `root["boards"].is<JsonArray>()`
- **Line 147**: `!root.containsKey("schedules")` -> `!root["schedules"].is<JsonArray>()`
- **Line 169**: `!root.containsKey("feeds")` -> `root["feeds"].isNull()`
- **Line 173, 174, 177**: Replace `containsKey` with `is<T>()` for `rssUrl`, `rssName`, and `weatherKeyId`.

### 📦 infrastructure

#### [MODIFY] [platformio.ini](platformio.ini)
- Update `olikraus/U8g2@2.36.5` -> `olikraus/U8g2@2.36.18`
- Update `bblanchon/ArduinoJson@7.2.0` -> `bblanchon/ArduinoJson@7.4.3`
- Modernize `mathieucarbou/ESPAsyncWebServer` to the latest maintained version (v3.7.0+) by the **ESP32Async** organization.

## Resource Impact Assessment

- **Memory (Flash)**: Negligible. Binaries may shrink slightly due to fewer methods called and library optimizations.
- **Memory (RAM/Heap)**: Improved efficiency in ArduinoJson 7.4.x and updated AsyncTCP buffers in the WebServer.
- **Performance**: High (Minor). Eliminates redundant dictionary lookups in configuration parsing.
- **Security**: Upgraded to latest TLS/TCP fixes in the underlying Async stacks.
- **Compatibility**: Maintains full support for legacy configuration versions.

## Verification Plan

### Automated Tests
- **Compilation**: Trigger a fresh build to ensure all `-Wdeprecated-declarations` warnings are resolved.
  ```bash
  pio run -e esp32s3nano
  ```

### Manual Verification
- **Web Portal**: Load the web portal (`/web`) to verify that the SPA assets are still served correctly (testing the `beginResponse` change).
- **Configuration Migration**: Verify that booting with a legacy `config.json` (e.g., v1.x or v2.x) still triggers the migration logic correctly.
