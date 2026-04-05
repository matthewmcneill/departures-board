# Settings Backup & Restore Implementation

I have successfully implemented the backup and restore functionality for the board configuration. This allows you to download your current layouts and settings as a JSON file and restore them later, which is ideal for factory reset testing.

## Summary of Changes

### C++ Backend
- **New Endpoints**: Registered `GET /api/config/backup` and `POST /api/config/restore`.
- **Backup Handler**: Serves the current `/config.json` as a file attachment.
- **Restore Handler**: Receives a JSON payload, validates it, overwrites `/config.json`, and triggers a system reload via `ConfigManager::requestReload()`.

### Web Portal (UI)
- **Maintenance Tab**: Added a new "MTCE" tab in the bottom navigation bar.
- **Backup UI**: A single button to download the `config.json` file.
- **Restore UI**: A file picker and restore button to upload and apply a saved configuration.
- **Security Messaging**: Included a clear note that API Keys and WiFi credentials are NOT included for security.

### Testing & Verification
- **Local Verification**: Verified the UI and JS handlers using a local mock server and browser testing.
- **Compilation**: Successfully compiled the C++ backend using `pio run`.

## Visual Walkthrough

````carousel
![Maintenance Tab](/Users/mcneillm/.gemini/antigravity/brain/cdd45a36-edd8-43da-afb3-e37d8d1555fd/.system_generated/click_feedback/click_feedback_1775376552905.png)
<!-- slide -->
![Backup & Restore Details](/Users/mcneillm/.gemini/antigravity/brain/cdd45a36-edd8-43da-afb3-e37d8d1555fd/.system_generated/click_feedback/click_feedback_1775376560007.png)
````

> [!IMPORTANT]
> **Encryption & Security**: Because API keys and WiFi settings are encrypted using hardware-specific keys, they are excluded from the backup to prevent credential leaks or issues when restoring between different boards.

## Verification Results

### Automated Tests
- `pio run`: **PASSED** (all modules compiled and linked).
- Local Mock Server: **PASSED** (API routing and UI state handling verified).

### Manual Verification Required
- Flash the device and perform a full "Backup -> Factory Reset -> Restore" cycle to verify the LittleFS file persistence on actual hardware.
