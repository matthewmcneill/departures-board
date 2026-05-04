# Dependency & Architectural Audit Report

This report evaluates the current codebase against the established architectural principles:
1. **Library (`lib/`)**: A standalone functionality module that does **not** depend on `appContext` or downstream application-specific modules.
2. **Module (`modules/`)**: A group of functionality that **does** depend on `appContext` or application-specific interfaces.

## User Review Required

> [!WARNING]
> Please review the updated findings below. Based on your clarification that `iConfigurable` binds components directly to the `configManager` module, several more libraries have been flagged. 
> 
> We must decide whether to **Migrate** these libraries into `modules/`, or attempt a significant **Dependency Inversion** refactor to completely untether them from `configManager` (which would require decoupling the monolithic `Config` struct).

## Codebase Architectural Evaluation

### 1. `lib/` Rule Violations (Non-Standalone Libraries)
Libraries are meant to be pure infrastructure, devoid of domain-specific business logic or global application state knowledge. 
- *Issue 1*: `lib/otaUpdater`
  - It `#include <appContext.hpp>` and `<configManager.hpp>`. 
  - It maintains a direct pointer to the `appContext` orchestrator and executes domain-level `departuresBoard` logic (e.g., UI updates, firmware validation).
  - It depends on the `iConfigurable.hpp` interface.
- *Issue 2*: `lib/rssClient`
  - It directly references the global `appContext` extern instance in its `.cpp` file to invoke `appContext.getDataManager().requestPriorityFetch(this)`. 
  - It manipulates the global `MessagePool`. 
  - It depends on the `iConfigurable.hpp` interface.
- *Issue 3*: `lib/timeManager`
  - It implements the `iConfigurable` interface, establishing a direct dependency on the `modules/configManager` domain. By definition, this prevents it from being a standalone infrastructure library since it is wired directly into the application's config schema.

**Recommendation**: The simplest enforcement of the principles is to move `otaUpdater`, `rssClient`, and `timeManager` sequentially into the `modules/` directory, as they are fundamentally tied to the application's configuration and `appContext` orchestrating systems. 

### 2. `modules/` Rule Violations (Standalone Modules)
Modules are expected to be context-aware logic blocks that interact with `appContext` or application configurations.
- **No Violations Detected**: As previously discussed, while `wifiManager` lacks an explicit `#include <appContext.hpp>`, its implementation of `iConfigurable` inextricably tethers it to the `configManager` module. It is correctly classified as an application module.

### 3. Validated Components (Compliant)
- **Compliant Libraries**: `boardLED`, `buttonHandler`, `deviceCrypto`, `githubClient`, `hTTPUpdateGitHub`, `logger`, `md5Utils`, `xmlListener`, `xmlStreamingParser` are successfully isolated and have no cross-contamination with `appContext` or `configManager`.
- **Compliant Modules**: `appContext`, `configManager`, `dataManager`, `displayManager`, `mcpServer`, `schedulerManager`, `weatherClient`, `webServer`, `wifiManager` correctly consume/depend on the application state.

## Proposed Refactoring Roadmap

### Phase 1: Structural Migration
1. Relocate `lib/otaUpdater` -> `modules/otaUpdateManager` (Renaming folder, `.cpp`, `.hpp` files, and internal classes to `otaUpdateManager`)
2. Relocate `lib/rssClient` -> `modules/rssClient`
3. Relocate `lib/timeManager` -> `modules/timeManager`
4. Update associated `#include` paths referencing these components across the entire codebase.
5. Update `CMakeLists.txt` or `platformio.ini` include directives if they explicitly point to these direct library paths for compilation.

### Phase 2: #include Cleanup & Validation
6. **Redundancy Scrub**: Traverse all files flagged in the first audit table and remove all duplicate in-file headers and redundant `.cpp` headers already supplied by the local `.hpp`.
7. **Unused Scrub**: Traverse all files flagged in the second audit table and selectively remove potentially unused headers.
8. **Compilation Validation**: Rigorously compile both `esp32dev` and `esp32s3nano` boards to ensure the scrubber did not accidentally remove a structurally necessary header.

## Open Questions

> [!IMPORTANT]
> **Direction Question**: Do you approve of migrating `otaUpdater`, `rssClient`, and `timeManager` into `modules/` to quickly cleanly resolve the definitions, or should we attempt the heavy lifting required to decouple `iConfigurable` entirely out of them so they can remain legitimate libraries?

---

## Detailed Includes Rigorous Audit

A deep static analysis was executed traversing every library and module to locate duplicate and redundant includes (e.g. including `<Arduino.h>` manually in a `.cpp` file when its corresponding `.hpp` already injects it).

| Component Directory | File | Audit Findings |
|---|---|---|
| `lib/boardLED` | `boardLED.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `lib/boardLED` | `boardLED.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `lib/buttonHandler` | `buttonHandler.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `lib/buttonHandler` | `buttonHandler.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `lib/deviceCrypto` | `deviceCrypto.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `lib/deviceCrypto` | `deviceCrypto.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `lib/githubClient` | `githubClient.cpp` | ⚠️ Redundant (already in githubClient.hpp): <JsonListener.h>, <md5Utils.hpp> |
| `lib/githubClient` | `githubClient.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `lib/hTTPUpdateGitHub` | `HTTPUpdateGitHub.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `lib/hTTPUpdateGitHub` | `hTTPUpdateGitHub.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `lib/logger` | `logger.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `lib/logger` | `logger.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `lib/md5Utils` | `md5Utils.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `lib/md5Utils` | `md5Utils.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `lib/otaUpdater` | `otaUpdater.cpp` | ⚠️ Redundant (already in otaUpdater.hpp): <githubClient.hpp> |
| `lib/otaUpdater` | `otaUpdater.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `lib/rssClient` | `rssClient.cpp` | ⚠️ Redundant (already in rssClient.hpp): <xmlListener.hpp> |
| `lib/rssClient` | `rssClient.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `lib/timeManager` | `timeManager.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `lib/timeManager` | `timeManager.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `lib/xmlListener` | `xmlListener.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `lib/xmlListener` | `xmlListener.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `lib/xmlStreamingParser` | `xmlStreamingParser.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `lib/xmlStreamingParser` | `xmlStreamingParser.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/appContext` | `appContext.cpp` | ⚠️ Redundant (already in appContext.hpp): <memory>, <wifiManager.hpp> |
| `modules/appContext` | `appContext.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/configManager` | `configManager.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/configManager` | `configManager.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/configManager` | `gadecMigration.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/configManager` | `gadecMigration.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/configManager` | `iConfigurable.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/dataManager` | `dataManager.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/dataManager` | `dataManager.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/dataManager` | `iDataSource.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager` | `boardFactory.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager` | `boardFactory.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager` | `displayManager.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager` | `displayManager.hpp` | ❌ Duplicate within file: "departuresBoard.hpp" |
| `modules/displayManager/boards/busBoard` | `busBoard.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/busBoard` | `busBoard.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/busBoard` | `busDataSource.cpp` | ⚠️ Redundant (already in busDataSource.hpp): "../../../dataManager/iDataSource.hpp", <memory> |
| `modules/displayManager/boards/busBoard` | `busDataSource.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/busBoard` | `iBusLayout.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/busBoard` | `iBusLayout.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/busBoard/layouts` | `layoutDefault.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/busBoard/layouts` | `layoutDefault.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/interfaces` | `iBoardLayout.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/interfaces` | `iDisplayBoard.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/nationalRailBoard` | `iNationalRailDataProvider.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/nationalRailBoard` | `iNationalRailLayout.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/nationalRailBoard` | `iNationalRailLayout.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/nationalRailBoard` | `nationalRailBoard.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/nationalRailBoard` | `nationalRailBoard.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/nationalRailBoard` | `nrDARWINDataProvider.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/nationalRailBoard` | `nrDARWINDataProvider.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/nationalRailBoard` | `nrRDMDataProvider.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/nationalRailBoard` | `nrRDMDataProvider.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/nationalRailBoard/layouts` | `layoutDefault.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/nationalRailBoard/layouts` | `layoutDefault.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/nationalRailBoard/layouts` | `layoutGadec.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/nationalRailBoard/layouts` | `layoutGadec.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/nationalRailBoard/layouts` | `layoutReplica.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/nationalRailBoard/layouts` | `layoutReplica.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/nationalRailBoard/layouts` | `layoutSWR.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/nationalRailBoard/layouts` | `layoutSWR.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/systemBoard` | `diagnosticBoard.cpp` | ⚠️ Redundant (already in diagnosticBoard.hpp): "layouts/layoutDiagnostic.hpp" |
| `modules/displayManager/boards/systemBoard` | `diagnosticBoard.cpp` | ❌ Duplicate within file: <fonts/fonts.hpp> |
| `modules/displayManager/boards/systemBoard` | `diagnosticBoard.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/systemBoard` | `firmwareUpdateBoard.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/systemBoard` | `firmwareUpdateBoard.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/systemBoard` | `helpBoard.cpp` | ⚠️ Redundant (already in helpBoard.hpp): "../../widgets/drawingPrimitives.hpp" |
| `modules/displayManager/boards/systemBoard` | `helpBoard.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/systemBoard` | `loadingBoard.cpp` | ⚠️ Redundant (already in loadingBoard.hpp): "../../widgets/drawingPrimitives.hpp" |
| `modules/displayManager/boards/systemBoard` | `loadingBoard.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/systemBoard` | `messageBoard.cpp` | ⚠️ Redundant (already in messageBoard.hpp): "../../widgets/drawingPrimitives.hpp" |
| `modules/displayManager/boards/systemBoard` | `messageBoard.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/systemBoard` | `sleepingBoard.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/systemBoard` | `sleepingBoard.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/systemBoard` | `splashBoard.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/systemBoard` | `splashBoard.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/systemBoard` | `wizardBoard.cpp` | ⚠️ Redundant (already in wizardBoard.hpp): "../../widgets/drawingPrimitives.hpp" |
| `modules/displayManager/boards/systemBoard` | `wizardBoard.cpp` | ❌ Duplicate within file: <fonts/fonts.hpp> |
| `modules/displayManager/boards/systemBoard` | `wizardBoard.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/systemBoard/layouts` | `layoutDiagnostic.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/systemBoard/layouts` | `layoutDiagnostic.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/tflBoard` | `iTflLayout.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/tflBoard` | `iTflLayout.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/tflBoard` | `tflBoard.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/tflBoard` | `tflBoard.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/tflBoard` | `tflDataSource.cpp` | ⚠️ Redundant (already in tflDataSource.hpp): "../../../dataManager/iDataSource.hpp", <memory> |
| `modules/displayManager/boards/tflBoard` | `tflDataSource.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/tflBoard/layouts` | `layoutDefault.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/boards/tflBoard/layouts` | `layoutDefault.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/fonts` | `fonts.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/fonts` | `fonts.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/messaging` | `messagePool.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/messaging` | `messagePool.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/widgets` | `clockWidget.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/widgets` | `clockWidget.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/widgets` | `drawingPrimitives.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/widgets` | `drawingPrimitives.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/widgets` | `iGfxWidget.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/widgets` | `imageWidget.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/widgets` | `imageWidget.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/widgets` | `labelWidget.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/widgets` | `labelWidget.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/widgets` | `locationAndFiltersWidget.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/widgets` | `locationAndFiltersWidget.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/widgets` | `progressBarWidget.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/widgets` | `progressBarWidget.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/widgets` | `scrollingMessagePoolWidget.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/widgets` | `scrollingMessagePoolWidget.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/widgets` | `scrollingTextWidget.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/widgets` | `scrollingTextWidget.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/widgets` | `serviceListWidget.cpp` | ⚠️ Redundant (already in serviceListWidget.hpp): "drawingPrimitives.hpp" |
| `modules/displayManager/widgets` | `serviceListWidget.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/widgets` | `systemMessageWidget.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/widgets` | `systemMessageWidget.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/widgets` | `trainFormationWidget.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/widgets` | `trainFormationWidget.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/widgets` | `weatherWidget.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/widgets` | `weatherWidget.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/widgets` | `wifiStatusWidget.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/displayManager/widgets` | `wifiStatusWidget.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/mcpServer` | `mcpServer.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/mcpServer` | `mcpServer.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/schedulerManager` | `schedulerManager.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/schedulerManager` | `schedulerManager.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/weatherClient` | `weatherClient.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/weatherClient` | `weatherClient.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/weatherClient` | `weatherStatus.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/webServer` | `portalAssets.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/webServer` | `webHandlerManager.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/webServer` | `webHandlerManager.hpp` | ✅ Passed (No, duplicates or redundant includes) |
| `modules/webServer` | `webServer.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/webServer` | `webServer.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `modules/wifiManager` | `wifiManager.cpp` | ⚠️ Redundant (already in wifiManager.hpp): <Arduino.h> |
| `modules/wifiManager` | `wifiManager.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `src` | `buildTime.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `src` | `buildTime.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `src` | `departuresBoard.cpp` | ✅ Passed (No duplicates or redundant includes) |
| `src` | `departuresBoard.hpp` | ✅ Passed (No duplicates or redundant includes) |
| `src` | `publicKey.hpp` | ✅ Passed (No duplicates or redundant includes) |

---

## Unused Includes Audit

A secondary static symbol mapping pass was performed to identify files containing `#include` directives but zero matching references to the symbols exported by that header (e.g. including `<ArduinoJson.h>` without actually declaring any `JsonDocument` in the file).

| Component Directory | File | Audit Findings |
|---|---|---|
| `lib/githubClient` | `githubClient.cpp` | 🔍 Potentially unused include: <HTTPClient.h> (No known exported symbols like 'HTTPClient' referenced) |
| `lib/githubClient` | `githubClient.cpp` | 🔍 Potentially unused include: <LittleFS.h> (No known exported symbols like 'LittleFS' referenced) |
| `lib/hTTPUpdateGitHub` | `HTTPUpdateGitHub.cpp` | 🔍 Potentially unused include: <WiFiClientSecure.h> (No known exported symbols like 'WiFiClientSecure' referenced) |
| `lib/otaUpdater` | `otaUpdater.cpp` | 🔍 Potentially unused include: <HTTPClient.h> (No known exported symbols like 'HTTPClient' referenced) |
| `lib/otaUpdater` | `otaUpdater.cpp` | 🔍 Potentially unused include: <appContext.hpp> (No known exported symbols like 'appContext' referenced) |
| `lib/otaUpdater` | `otaUpdater.hpp` | 🔍 Potentially unused include: <LittleFS.h> (No known exported symbols like 'LittleFS' referenced) |
| `lib/otaUpdater` | `otaUpdater.hpp` | 🔍 Potentially unused include: <WiFiClientSecure.h> (No known exported symbols like 'WiFiClientSecure' referenced) |
| `lib/rssClient` | `rssClient.cpp` | 🔍 Potentially unused include: <xmlListener.hpp> (No known exported symbols like 'xmlListener' referenced) |
| `lib/rssClient` | `rssClient.hpp` | 🔍 Potentially unused include: <xmlStreamingParser.hpp> (No known exported symbols like 'xmlStreamingParser' referenced) |
| `lib/xmlListener` | `xmlListener.hpp` | 🔍 Potentially unused include: <Arduino.h> (No known exported symbols like 'pinMode' referenced) |
| `modules/appContext` | `appContext.cpp` | 🔍 Potentially unused include: <wifiManager.hpp> (No known exported symbols like 'WiFiState' referenced) |
| `modules/configManager` | `configManager.hpp` | 🔍 Potentially unused include: <ArduinoJson.h> (No known exported symbols like 'DeserializationError' referenced) |
| `modules/configManager` | `gadecMigration.cpp` | 🔍 Potentially unused include: <Arduino.h> (No known exported symbols like 'pinMode' referenced) |
| `modules/displayManager` | `displayManager.cpp` | 🔍 Potentially unused include: <weatherClient.hpp> (No known exported symbols like 'weatherClient' referenced) |
| `modules/displayManager/boards/busBoard` | `busBoard.hpp` | 🔍 Potentially unused include: <layoutDefault.hpp> (No known exported symbols like 'layoutNrDefault' referenced) |
| `modules/displayManager/boards/busBoard` | `busDataSource.cpp` | 🔍 Potentially unused include: <HTTPClient.h> (No known exported symbols like 'HTTPClient' referenced) |
| `modules/displayManager/boards/nationalRailBoard` | `nationalRailBoard.hpp` | 🔍 Potentially unused include: <layoutDefault.hpp> (No known exported symbols like 'layoutNrDefault' referenced) |
| `modules/displayManager/boards/nationalRailBoard` | `nationalRailBoard.hpp` | 🔍 Potentially unused include: <layoutReplica.hpp> (No known exported symbols like 'layoutNrReplica' referenced) |
| `modules/displayManager/boards/systemBoard` | `helpBoard.hpp` | 🔍 Potentially unused include: <drawingPrimitives.hpp> (No known exported symbols like 'TextAlign' referenced) |
| `modules/displayManager/boards/systemBoard` | `loadingBoard.hpp` | 🔍 Potentially unused include: <drawingPrimitives.hpp> (No known exported symbols like 'TextAlign' referenced) |
| `modules/displayManager/boards/systemBoard` | `loadingBoard.hpp` | 🔍 Potentially unused include: <imageWidget.hpp> (No known exported symbols like 'imageWidget' referenced) |
| `modules/displayManager/boards/systemBoard` | `messageBoard.hpp` | 🔍 Potentially unused include: <drawingPrimitives.hpp> (No known exported symbols like 'TextAlign' referenced) |
| `modules/displayManager/boards/systemBoard` | `sleepingBoard.cpp` | 🔍 Potentially unused include: <drawingPrimitives.hpp> (No known exported symbols like 'TextAlign' referenced) |
| `modules/displayManager/boards/systemBoard` | `splashBoard.hpp` | 🔍 Potentially unused include: <drawingPrimitives.hpp> (No known exported symbols like 'TextAlign' referenced) |
| `modules/displayManager/boards/systemBoard` | `splashBoard.hpp` | 🔍 Potentially unused include: <progressBarWidget.hpp> (No known exported symbols like 'progressBarWidget' referenced) |
| `modules/displayManager/boards/systemBoard` | `wizardBoard.cpp` | 🔍 Potentially unused include: <appContext.hpp> (No known exported symbols like 'appContext' referenced) |
| `modules/displayManager/boards/systemBoard` | `wizardBoard.hpp` | 🔍 Potentially unused include: <WiFi.h> (No known exported symbols like 'WiFi' referenced) |
| `modules/displayManager/boards/systemBoard` | `wizardBoard.hpp` | 🔍 Potentially unused include: <drawingPrimitives.hpp> (No known exported symbols like 'TextAlign' referenced) |
| `modules/displayManager/boards/tflBoard` | `tflBoard.hpp` | 🔍 Potentially unused include: <layoutDefault.hpp> (No known exported symbols like 'layoutNrDefault' referenced) |
| `modules/displayManager/widgets` | `progressBarWidget.cpp` | 🔍 Potentially unused include: <displayManager.hpp> (No known exported symbols like 'appContext' referenced) |
| `modules/displayManager/widgets` | `serviceListWidget.cpp` | 🔍 Potentially unused include: <displayManager.hpp> (No known exported symbols like 'appContext' referenced) |
| `modules/displayManager/widgets` | `weatherWidget.cpp` | 🔍 Potentially unused include: <weatherClient.hpp> (No known exported symbols like 'weatherClient' referenced) |
| `modules/schedulerManager` | `schedulerManager.hpp` | 🔍 Potentially unused include: <configManager.hpp> (No known exported symbols like 'ScheduleRule' referenced) |
| `modules/webServer` | `webHandlerManager.cpp` | 🔍 Potentially unused include: <wifiManager.hpp> (No known exported symbols like 'WiFiState' referenced) |
| `modules/webServer` | `webServer.cpp` | 🔍 Potentially unused include: <Arduino.h> (No known exported symbols like 'pinMode' referenced) |
| `modules/wifiManager` | `wifiManager.cpp` | 🔍 Potentially unused include: <deviceCrypto.hpp> (No known exported symbols like 'DeviceCrypto' referenced) |
