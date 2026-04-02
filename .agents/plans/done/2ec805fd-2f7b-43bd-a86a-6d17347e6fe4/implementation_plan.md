# Archiving Legacy Web Infrastructure & Renaming Portal

This plan outlines the steps to move the old web page infrastructure to an `archive` folder, remove them from the main compilation process, and rename the modern portal from `/portal` to `/web`.

## User Review Required

> [!IMPORTANT]
> This change will deactivate all legacy web routes (e.g., `/`, `/index.htm`, `/config.json`, `/info`, etc.) and redirect the root `/` to the modern `/web`.
> [!WARNING]
> Manual firmware update (`/update`) and file upload (`/upload`) tools from the old codebase will also be archived. Ensure these are not required for your current workflow.

## Proposed Changes

### Build System & Directory Structure

#### [NEW] [archive/](archive)
Create an `archive` directory in the repository root to house decommissioned components.

#### [MOVE] [web/](web) -> [archive/web/](archive/web)
Move the legacy HTML/JSON assets to the archive.

#### [MOVE] [portal/](portal) -> [web/](web)
Rename the modern portal source directory to `web/` (replacing the archived directory).

#### [MOVE] [include/webgui/](include/webgui) -> [archive/include/webgui/](archive/include/webgui)
Move the legacy C++ headers containing embedded web assets and handlers.

#### [MODIFY] [portalBuilder.py](scripts/portalBuilder.py)
Update `PORTAL_DIR` to point to the new `web/` directory.

---

### Core Modules (C++)

#### [MODIFY] [departuresBoard.hpp](src/departuresBoard.hpp)
- Define `VERSION_MAJOR` and `VERSION_MINOR` here as `#define` constants.
- Firmware and Web UI will now share this unified version since they are bundled together.

#### [MODIFY] [departuresBoard.cpp](src/departuresBoard.cpp)
- Remove global `int VERSION_MAJOR` and `VERSION_MINOR` variables (replaced by header defines).

#### [MODIFY] [otaUpdater.cpp](lib/otaUpdater/otaUpdater.cpp)
- Remove inclusion of legacy `../../include/webgui/index.h`.
- Update `checkPostWebUpgrade()` to use unified `VERSION_MAJOR`/`MINOR` for asset cleanup tracking.
- Add stubs/comments describing legacy OTA portal interactions (`/ota` and `/info` behavior) to facilitate future integration.

#### [MODIFY] [otaUpdater.hpp](lib/otaUpdater/otaUpdater.hpp)
- Remove `extern int VERSION_MAJOR` and `VERSION_MINOR` declarations (now picked up via `#include <departuresBoard.hpp>`).

#### [MODIFY] [webHandlerManager.cpp](modules/webServer/webHandlerManager.cpp)
- Update registration routes: change `/portal` to `/web`.
- Update logging and captive portal redirect to use `/web`.
- Rename all `/portal` route registrations to `/web`.
- Update logging and captive portal redirects to use the new `/web` path.

#### [MODIFY] [webServer.cpp](modules/webServer/webServer.cpp)
- Remove all includes pointing to `../../include/webgui/`.
- Remove registration of legacy routes (lines 50-69, 72-100).
- Add a permanent redirect for `/` to `/web`.
- Remove the direct inclusion of `WebHandlers.hpp`.
- Remove all legacy includes and route registrations.
- Retain only core server management and modern `WebHandlerManager`.
- Implement a redirect from `/` to `/web` for user convenience.

---

### Testing & Verification

#### [MODIFY] [test/web/](test/web)
Update Playwright tests and local dev server (`server.js`) to reflect the rename from `/portal` to `/web`.

## Verification Plan

### Automated Tests
- Run updated web portal tests to ensure the modern portal (now at `/web`) and its API still function correctly.
  ```bash
  cd test/web && npx playwright test
  ```

### Manual Verification
- Compile and flash the device.
- Navigate to the root IP address (e.g., `http://192.168.1.x/`) and verify it redirects to `/web`.
- Verify the modern portal fully loads and functions (fetching status, saving config).
- Attempt to access legacy routes like `/info` or `/config.json` and verify they return 404 or redirect.
