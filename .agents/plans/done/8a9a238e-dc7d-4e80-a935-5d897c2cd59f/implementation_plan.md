[x] Reviewed by house-style-documentation - passed
[x] Reviewed by architectural-refactoring - passed
[x] Reviewed by embedded-systems - passed, memory footprint unaffected as only include scopes change.

# Include Graph Optimization Execution

Full execution of pruning global `.hpp` directives to shift `LittleFS` and `WiFi` dependencies strictly to target implementation scopes.

## User Review Required

> [!IMPORTANT]
> The original `implementation_plan.md` was missing from the plan directory (`8a9a238e`). I have reconstructed this implementation plan from the saved `task.md` and `walkthrough.md` artifacts. Please review to ensure it correctly captures the intended optimizations. Note that based on the task checklist, all of these steps were marked as previously completed.

## Proposed Changes

### Configuration Manager

#### [MODIFY] configManager.hpp
- Remove `#include <LittleFS.h>` to prevent global propagation.
- Remove `#include <WiFi.h>` as it is not needed in the interface.

#### [MODIFY] configManager.cpp
- Inject `#include <LittleFS.h>` explicitly to fulfill dependency requirements for internal `LittleFS.open()` calls.

---

### System Manager

#### [MODIFY] systemManager.hpp
- Strip out the unnecessary `#include <WiFi.h>`.

---

### Web Server

#### [MODIFY] webServer.cpp
- Remove obsolete legacy inclusion of `#include <SPIFFS.h>`.

---

### Departures Board Core

#### [MODIFY] departuresBoard.cpp
- Remove duplicate inclusion of `#include "departuresBoard.hpp"`.

## Open Questions

- Since the saved `task.md` marked all of these items as `[x]`, are we reapplying these changes to a fresh branch, or are there any additional headers we need to prune during this session?

## Verification Plan

### Automated Tests
- Run native compilation via `pio run -e esp32dev` to ensure no circular dependency problems or undeclared identifiers.

### Manual Verification
- Verify the PlatformIO build completes without errors, confirming the header dependencies resolve locally.
