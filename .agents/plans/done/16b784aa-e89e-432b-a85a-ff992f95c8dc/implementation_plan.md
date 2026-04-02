# Dynamic Default Tab Selection

This change introduces a "smart" initial tab selection for the web configuration portal. Instead of always starting on the "WiFi" tab, the application will assess the device's state upon loading and guide the user to the most relevant section.

## Proposed Changes

### [Web Portal]

#### [MODIFY] [index.html](web/index.html)
- Add a flag `initialTabSet` to the `app.state` to prevent jumping tabs after the initial load.
- Implement a `determineInitialTab()` logic within `app.init()` or as a callback after `fetchConfig` and `fetchStatus` complete.
- Logic:
    1. If `status.ap_mode` is true OR `!status.connected`: Open **WiFi** tab.
    2. Else if no keys (no tokens) AND no displays: Open **Keys** tab. (This guides new users to set up a key before a display).
    3. Else: Open **Displays** tab. (This is the primary dashboard for users with an active setup).

## Verification Plan

### Automated Tests
- No automated tests available for frontend UI logic in this environment.

### Manual Verification
1. **Case: No WiFi**
    - Boot device in AP mode (or disconnect from WiFi).
    - Open the portal.
    - Verify it opens on the **WiFi** tab.
2. **Case: WiFi OK, No Keys, No Displays**
    - Connect device to WiFi but ensure `config.keys` has no tokens and `config.boards` is empty.
    - Open the portal.
    - Verify it opens on the **Keys** tab.
3. **Case: WiFi OK, Keys/Displays exist**
    - Configure at least one key or board.
    - Open the portal.
    - Verify it opens on the **Displays** tab.
4. **Case: Navigation Persistence**
    - Once the portal is open, navigate to "System" tab.
    - Wait for a background refresh (5s).
    - Verify the tab *does not* jump back to the default.
