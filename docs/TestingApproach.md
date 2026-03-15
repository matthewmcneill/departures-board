# Testing Approach

This document outlines the testing infrastructure for the Departures Board project. The goal is to ensure reliability of both the web-based configuration portal and the core C++ firmware logic.

## Overview

The project uses a two-pronged testing strategy:
1.  **Web Portal E2E Tests**: Driven by [Playwright](https://playwright.dev/), ensuring the configuration UI and its logic function correctly.
2.  **C++ Unit Tests**: Driven by [Unity](http://www.throwtheswitch.org/unity) and PlatformIO's native environment, allowing logic to be tested on the development host.

---

## Web Portal Testing (Playwright)

Located in `test/web/`, these tests verify the Single Page Application (SPA) that configures the board.

### 1. Local Mocked Tests
These tests run against the local source code of the portal using a mock server.
-   **Infrastructure**: A Node.js server (`server.js`) serves the portal while Playwright intercepts `/api/*` calls.
-   **Philosophy**: Focuses on **Software Logic**. We can test destructive actions (Deleting keys), error states (Invalid tokens), and slow operations (WiFi scanning) instantly and without risk.
-   **Scope**: 9+ tests covering full CRUD, validation feedback, and UI state transitions.

### 2. Live Hardware Tests
These tests verify the portal as served by the real ESP32 hardware.
-   **Infrastructure**: Targets the real device IP (e.g., `BASE_URL=http://192.168.86.152/portal`).
-   **Philosophy**: Focuses on **Hardware Integration**. Proves the server is up, assets are correctly flashed, and real hardware APIs (IP, MAC, RSSI) feed the UI correctly.
-   **Scope & Safety**: Limited to "Smoke Tests" (3+ tests). We avoid destructive actions (Deletion) to preserve real config, and we avoid WiFi connection tests which would disconnect the test runner from the device.

### 3. Build System Integration
Automated local tests are integrated into the **PlatformIO** build process:
-   A pre-script (`scripts/run_web_tests.py`) is triggered before every firmware build for `esp32dev` and `esp32s3nano`.
-   This script runs the mocked local tests to ensure no regressions were introduced to the portal logic.
-   **Skipping**: Set `SKIP_WEB_TESTS=1` in your environment to bypass these tests if they are slowing down your iterative build cycle.

---

## C++ Unit Testing

Located in `test/test_native/`, these tests verify C++ classes and modules that do not depend on hardware APIs (or can have those APIs mocked).

### Native Environment
We use the `native` environment in `platformio.ini` to compile and run tests on your Mac:
```bash
pio test -e native
```
Tests are written using the Unity framework. Avoid hardware-specific headers (like `Arduino.h`) in these tests unless you provide mocks.

---

## How to Add Tests

### Adding Web Tests
1.  Add a new `.spec.ts` file in `test/web/tests/`.
2.  Refrence existing tests in `portal.spec.ts` for mocking patterns.
3.  Ensure your test uses `await page.goto('/')` if testing locally, or `await page.goto(process.env.BASE_URL)` for live tests.

### Adding C++ Tests
1.  Add a new test directory in `test/`. PlatformIO expects each test suite to have its own folder if using multiple files, or you can add files to `test/test_native/`.
2.  Include `unity.h`.
3.  Implement `setUp()`, `tearDown()`, and your test functions.
4.  Call `RUN_TEST(func)` in `main()`.

---

## How to Run Tests Manually

### Web Portal (Playwright)
```bash
cd test/web
npm install          # If dependencies changed
npx playwright test  # Run all tests (including mocked local)
```

### C++ Unit Tests
```bash
# Run all native tests
pio test -e native

# Run a specific test suite
pio test -e native -f test_native
```
