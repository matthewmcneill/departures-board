---
description: Read the web testing strategy rules. The mandatory 3-phase testing workflow for all web portal development in this project.
---
# Web Testing Strategy Rule

This rule defines the mandatory 3-phase testing workflow for all web portal development in this project. All agent sessions MUST follow this sequence to ensure rapid iteration while maintaining high quality and avoiding unnecessary hardware cycles.

## Phase 1: Local Development & Iteration

**Goal**: Rapidly iterate on UI, UX, and logic using a native test harness.

### Requirements
- You MUST perform initial development using the local web server and Playwright test suite.
- **Location**: `test/web`
- **Commands**:
    - Start the dev server: `node server.js`
    - Run automated tests: `npm test` (or `npx playwright test`)
- **Action**: Fix all UI issues and logic bugs in this phase. Do NOT move to Phase 2 until local tests are stable.

## Phase 2: Target Compilation & Integrated Build

**Goal**: Verify that the portal assets are correctly minified, gzipped, and integrated into the C++ firmware.

### Requirements
- You MUST execute a full PlatformIO build to trigger the automated build pipeline.
- **Pipeline**:
    1. `scripts/portalBuilder.py`: Minifies and gzips `portal/index.html` into `include/webServer/portalAssets.h`.
    2. `scripts/run_web_tests.py`: Automatically runs Playwright tests against the build directory to ensure no regressions were introduced by minification.
- **Commands**:
    - Build: `pio run`
- **Action**: Ensure the build completes without errors. If `run_web_tests.py` fails, you must return to Phase 1 or fix the builder scripts.

## Phase 3: Hardware Validation (Flash)

**Goal**: Final verification of the portal on real hardware.

### Requirements
- You MUST flash the device only AFTER Phase 2 is successful.
- **Pre-requisite**: Check `.agents/plans/lock.md` for hardware availability (refer to `queue-enforcement.md`).
- **Commands**:
    - Flash & Monitor safely: Execute the `/flash-test` workflow natively.
    - Check the system stability: Execute the `/read-flash-logs` workflow.
- **Action**: Perform a final smoke test of the portal in a real browser directed at the device's IP address.

## Rationale
- **Phase 1** is the fastest (seconds).
- **Phase 2** ensures the build system and asset pipeline are intact (minutes).
- **Phase 3** is the slowest and relies on shared hardware (minutes + potential queue wait).

By following this strategy, we minimize "blind flashing" and ensure that only verified code reaches the device.
