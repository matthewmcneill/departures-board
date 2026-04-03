# Implementation Plan: Secure Password/Key Placeholder Pattern

Migrate the WiFi password and API key fields to the canonical "Placeholder Pattern" as defined in [PasswordFieldImplementationGuide.md](docs/PasswordFieldImplementationGuide.md).

## Skill Audits
- [x] Reviewed by `house-style-documentation` - passed
- [x] Reviewed by `architectural-refactoring` - passed
- [x] Reviewed by `embedded-systems` - passed

## User Review Required

> [!IMPORTANT]
> This change changes the REST API response for sensitive fields. The `token` and `pass` fields will now be EMPTY strings in GET responses. A new field `tokenExists` or `passExists` will be used to signal the presence of a secret to the UI.

> [!WARNING]
> This refactor removes the "Magic String" (`●●●●●●●●`) comparison logic. Existing frontend code that relies on this specific string will break until updated (all updates included in this plan).

## Proposed Changes

### **1. Backend: Core & Config**
Refactor the secret management to use existence flags instead of masked strings.

#### [MODIFY] [wifiManager.hpp](modules/wifiManager/wifiManager.hpp)
- Add `bool hasPassword() const` method.
- Deprecate/remove `getPassMasked()`.

#### [MODIFY] [wifiManager.cpp](modules/wifiManager/wifiManager.cpp)
- Implement `hasPassword()`.
- Update `updateWiFi` to only update if `pass` is non-empty (removing magic string check).

#### [MODIFY] [configManager.hpp](modules/configManager/configManager.hpp)
- Add `bool hasToken()` to `ApiKey` struct or `ConfigManager` helper.

### **2. Backend: Web API**
Update REST handlers to implement the new data flow.

#### [MODIFY] [webHandlerManager.cpp](modules/webServer/webHandlerManager.cpp)
- **GET `/api/config`**: Return `passExists: true` and `tokenExists: true` instead of masked strings.
- **POST `/api/saveall`**: Update WiFi logic to skip password update if field is missing or empty.
- **POST `/api/keys`**: Update `handleSaveKey` to skip token update if field is empty.
- **POST `/api/keys/test` & `/api/wifi/test`**: Use "existing" secret if the token/pass field is empty in the test request.

### **3. Frontend: Portal**
Implement the visual and logic changes in the web UI.

#### [MODIFY] [index.html](portal/index.html)
- **CSS**: Add `input.existing-password-mask::placeholder` styles for authentic dot spacing.
- **HTML**:
    - Update WiFi and Key token inputs to use `value=""` and dynamically set `placeholder="••••••••"` if `passExists`/`tokenExists` is true.
    - Add `autocomplete="new-password"` to sensitive inputs.
- **JS**:
    - Refactor `render()` and `renderKeys()` to handle `passExists`/`tokenExists`.
    - Update `bindEvents()` to enable/disable toggle button based on `input.value.length > 0`.
    - Update `saveAll()` and `saveKey()` to only send secrets if they have been modified.

---

### **Phase 2: Network Robustness**
Implement failover logic for prolonged WiFi disconnections.

#### [MODIFY] [displayManager.hpp](modules/displayManager/displayManager.hpp)
- Add `SYS_ERROR_WIFI` to `SystemBoardId` enum.

#### [MODIFY] [displayManager.cpp](modules/displayManager/displayManager.cpp)
- Add case for `SYS_ERROR_WIFI` in `getSystemBoard`, returning a `MessageBoard` with "WIFI DISCONNECTED" content.

#### [MODIFY] [systemManager.hpp](modules/systemManager/systemManager.hpp)
- Add `unsigned long wifiDisconnectTimer = 0;` and `bool isWifiPersistentError() const;`

#### [MODIFY] [systemManager.cpp](modules/systemManager/systemManager.cpp)
- In `tick()`:
    - If `WiFi.status() != WL_CONNECTED`, increment `wifiDisconnectTimer`.
    - If `wifiConnected` becomes true, reset `wifiDisconnectTimer` and set `nextDataUpdate = 0` for immediate refresh.
- Implement `isWifiPersistentError()` to return `true` if `wifiDisconnectTimer > 180000` (3 mins).

#### [MODIFY] [nationalRailBoard.cpp, tflBoard.cpp, busBoard.cpp](modules/boards/)
- In `render()`:
    - `if (context->getSystemManager().isWifiPersistentError()) { context->getDisplayManager().getSystemBoard(SystemBoardId::SYS_ERROR_WIFI)->render(display); return; }`
- In `renderAnimationUpdate()`:
    - `if (context->getSystemManager().isWifiPersistentError()) { return; }` // System board doesn't need anim updates.

#### [MODIFY] [wifiStatusWidget.cpp](modules/displayManager/widgets/wifiStatusWidget.cpp)
- Implement a slow blink (1s) for the disconnection icon if disconnected for > 30s to increase prominence.

---

### **Phase 4: API Key Sequential Validation**
Optimize the frontend to prevent resource contention on the ESP32.

#### [MODIFY] [index.html](portal/index.html)
- **JS**:
    - Add `this.runSequentialTests()` method that iterates through `this.state.keys` and calls `this.testKeyAsync()` for each, awaiting completion.
    - Update `renderKeys()` to NOT trigger `testKeyAsync` automatically.
    - Update `render()` to call `this.runSequentialTests()` after `renderKeys()`, specifically when the 'API Keys' tab is first loaded or on refresh.
    - Add logic to stagger requests with a 500ms delay between them.

---

## Verification Plan

### Automated Tests
- **Playwright Web Tests**:
    ```bash
    cd test/web
    npx playwright test
    ```
- **New Test Case**: Add a test in `portal.spec.ts` that verifies the `value` is empty even when a password "exists", and that the correct `placeholder` is shown.

### Manual Verification
1.  **WiFi Test**:
    - Navigate to WiFi tab.
    - Select a network (should show masked placeholder, empty value).
    - Click "Test Connection" without typing (should use stored password).
    - Enter a *new* password and click "Test Connection" (should use new password).
2.  **API Key Test**:
    - Navigate to Keys tab.
    - Edit an existing key.
    - Click "Test" without changing the token (should pass using stored token).
    - Change the token and click "Test" (should use new token).
3.  **UI/Password Visibility**:
    - Verify eye icon is disabled/hidden when field is empty (placeholder only).
    - Verify eye icon becomes active as soon as the first character is typed.
4.  **Sequential Test**:
    - Force refresh the portal on the Keys tab.
    - Verify that status dots pulse amber ONE BY ONE and turn green/red sequentially rather than all at once.
