---
name: "Web Portal Test Suite & API Key Verification"
description: "Implemented a comprehensive testing suite (Playwright for E2E web, Unity for C++) to ensure portal reliability. Resolved a critical portal unresponsiveness bug caused by minification corruption and mi..."
created: "2026-03-15"
status: "DONE"
commits: ['da2252b']
---

# Summary
Implemented a comprehensive testing suite (Playwright for E2E web, Unity for C++) to ensure portal reliability. Resolved a critical portal unresponsiveness bug caused by minification corruption and missing JS methods. Conducted an architectural review verifying the encapsulation of the API Key Registry based on embedded systems best practices.

## Key Decisions
- **Two-Pronged Testing**: Separated web tests into "Local Mocked Tests" (for logic and destructive actions) and "Live Hardware Tests" (for smoke testing against the actual ESP32).
- **Build Integration**: Added a pre-build Python python script (`run_web_tests.py`) to the PlatformIO toolchain to automatically run mocked UI tests before compiling firmware, preventing regressions from reaching hardware.
- **Minifier Tightening**: Updated `portalBuilder.py` to aggressively strip single-line comments and whitespace, reducing the embedded HTML asset from 31KB to 19KB.
- **Architectural Validation**: Confirmed that the API Key components correctly use abstraction (referencing `apiKeyId` rather than the text key), and that secrets remain isolated in `/apikeys.json` while being redacted from serial logging.

## Technical Context
