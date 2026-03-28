# Task Checklist: Secure Password & API Key Implementation

- [x] **Planning & Design**
    - [x] Read `PasswordFieldImplementationGuide.md`
    - [x] Evaluate current implementation in `portal/index.html` and `webHandlerManager.cpp`
    - [x] Consult architectural and house style rules
    - [x] Create `implementation_plan.md`
- [x] **Phase 1: Backend Refactoring (REST API Cleanup)**
    - [x] Update `Config` and `WifiManager` to support "secret existence" flags
    - [x] Update `WebHandlerManager` GET handlers to emit `tokenExists` / `passExists`
    - [x] Update `WebHandlerManager` POST handlers to support optional updates and test-existing logic
- [x] **Phase 2: Frontend Refactoring (Compliance)**
    - [x] Implement `existing-password-mask` CSS in `index.html`
    - [x] Update password fields in WiFi and Key sections to use placeholder pattern
    - [x] Implement JavaScript logic for enabling/disabling toggle icons and handling "empty" submissions
- [x] **Debugging WiFi Disconnection**
    - [x] Resolve conflict between SystemManager and WifiManager
    - [x] Implement native reconnection in WifiManager state machine
    - [x] Build and Flash
    - [x] Verify recovery on serial monitor
- [x] Verification of Network Robustness on hardware
- [x] Finalize walkthrough.md and documentation

## Phase 3: API Key Validation Debugging
- [x] Fix status dot CSS colors (green/red)
- [x] Implement robust POST body parsing for JSON endpoints (handle "No data" error)
- [x] Verify premium styling and validation state in portal
- [x] Phase 4: API Key Sequential Validation
    - [x] Implement `runSequentialTests` in `index.html`
    - [x] Refactor `renderKeys` to remove automatic parallel testing
    - [x] Trigger sequential tests on tab load/refresh
    - [x] Verify sequential dot updates in portal (locally updated & synced to portalAssets.h)
- [x] Final hardware verification (Verified on live device)
- [x] Wrap session (Log, walkthrough, documentation, and commit)
