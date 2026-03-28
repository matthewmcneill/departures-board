# Task: Remove bustimes.org button

- [x] Research and Planning [x]
    - [x] Locate button in `portal/index.html`
    - [x] Check for references to "bus" key type in other files [x]
- [x] Implementation [x]
    - [x] Remove `type-card` for "bus" from `portal/index.html`
    - [x] Remove "bus" from `PROVIDER_LOGOS` in `portal/index.html`
    - [x] Check if `app.addKey('bus')` logic needs updates
    - [x] Regenerated `include/webServer/portalAssets.h`
- [x] Verification [x]
    - [x] Verify the button is gone from the UI
    - [x] Ensure other buttons still work
    - [x] Run automated Playwright tests
