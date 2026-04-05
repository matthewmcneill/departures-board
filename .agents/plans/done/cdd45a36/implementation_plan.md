# [Goal Description]

Provide a section in the web portal to save and load the settings JSON file (`/config.json`). This will allow users to backup their configuration and restore it, which is particularly useful for factory reset testing.

## User Review Required

> [!IMPORTANT]
> **Credential Exclusion**: The backup will only include the layout and general settings stored in `/config.json`. API Keys and WiFi credentials, which are stored in encrypted binary blobs (`/apikeys.bin` and `/wifi.bin`), will **NOT** be included in this backup for security and hardware-bound reasons. Users must re-enter credentials after a full factory reset.

> [!WARNING]
> **Overwrite Warning**: Restoring a configuration will completely overwrite the existing board layouts and schedules. The device will automatically reload the configuration after a successful restore.

## Proposed Changes

---

### [C++ Backend]

#### [MODIFY] [webHandlerManager.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/webServer/webHandlerManager.cpp)
- Register new routes:
  - `GET /api/config/backup`: Serves `/config.json` as an attachment.
  - `POST /api/config/restore`: Receives JSON body, validates it, and saves to `/config.json`.
- Implement `handleBackupConfig`:
    - Read `/config.json` using `ConfigManager::loadFile`.
    - Send with `Content-Type: application/json`.
- Implement `handleRestoreConfig`:
    - Parse incoming JSON body.
    - Validate that it's a valid Config JSON (check for "version" or root keys).
    - Save to `/config.json` and call `_context.getConfigManager().requestReload()`.

---

### [Web Portal UI]

#### [MODIFY] [index.html](file:///Users/mcneillm/Documents/Projects/departures-board/web/index.html)
- Add a new "Maintenance" section in the UI (likely a new tab in the navigation).
- Add "Backup & Restore" card with:
    - **Backup Button**: Triggers a fetch to `/api/config/backup` and uses a hidden link to download the blob.
    - **Restore File Input**: Allows selecting a `.json` file.
    - **Restore Button**: Reads the file via `FileReader` and POSTs the content to `/api/config/restore`.
- Add clear UI feedback for success/failure of the restore operation.

---

### [Testing Harness]

#### [MODIFY] [server.js](file:///Users/mcneillm/Documents/Projects/departures-board/test/web/server.js)
- Mock the new `/api/config/backup` and `/api/config/restore` endpoints to allow local UI testing with Playwright.

---

## Open Questions

- Should we also provide a way to backup `apikeys.bin` and `wifi.bin` as raw blobs for "same-device" restoration? 
  - *My recommendation*: No, to keep it simple and avoid encouraging users to move encrypted blobs between devices where they might not work.

## Verification Plan

### Automated Tests
- **Phase 1: Local**:
    - `npm test -g "Maintenance"` (new Playwright test) to verify UI behavior, download trigger, and upload flow.
- **Phase 2: Build**:
    - `pio run` to verify the `portalBuilder.py` correctly minifies the updated `index.html`.

### Manual Verification
1.  Open the web portal on the device.
2.  Navigate to the new "Maintenance" section.
3.  Click "Download Backup" and verify `config.json` is saved.
4.  Modify some settings in the UI and save.
5.  Restore the previously downloaded `config.json`.
6.  Verify that boards and schedules are reverted to the backup state.
