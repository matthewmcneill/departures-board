# Autonomous Build & Validation Plan

This plan outlines the steps for an autonomous build and flash cycle, followed by serial monitoring and web-based validation of the recent technical debt refactoring.

## Audit Checklist
- [x] Reviewed by `house-style-documentation` - passed
- [x] Reviewed by `architectural-refactoring` - passed
- [x] Reviewed by `embedded-systems` - passed
- [x] Reviewed by `systematic-debugging` - passed

## Goal Description
The objective is to verify that the refactoring on the `refactor/technical-debt` branch (including lazy-loading, factory patterns, and boot progress improvements) has not introduced regressions in the firmware or the web portal.

## User Review Required
> [!IMPORTANT]
> This plan requires access to the physical ESP32 hardware. The hardware is currently **LOCKED** by session `bef17266-675a-4ead-b50a-c52b44bfc34a`. This session will either wait for the lock to be released or the user must use `/queue-release` if that session is defunct.

> [!WARNING]
> Flashing the device will interrupt any current operations. Ensure the device is connected and in a safe state for flashing.

## Proposed Changes
No source code changes are proposed in this plan. This is a validation and testing workflow.

### [Phase 1] Local Web Validation
Perform initial smoke tests on the web portal assets using the local test harness.
- **Path**: `test/web`
- **Actions**:
    - Start local server: `node server.js`
    - Run automated tests: `npx playwright test`

### [Phase 2] Firmware Compilation & Pipeline
Compile the firmware and verify the automated asset pipeline (minification, compression, and unit testing).
- **Actions**:
    - Claim Hardware Lock using `/plan-start`.
    - Build: `pio run`
    - Verify `scripts/portalBuilder.py` and `scripts/run_web_tests.py` execution.

### [Phase 3] Hardware Deployment & Serial Monitoring
Flash the firmware and monitor the boot sequence for stability.
- **Actions**:
    - Flash: `pio device upload`
    - Monitor: `pio device monitor`
    - **Validation**: Confirm the 0-100% progress bar renders without deadlocks.

### [Phase 4] Remote Browser Validation
Perform a final functional verification of the web portal running on the real hardware.
- **Actions**:
    - Identify device IP from serial logs.
    - Use browser subagent to navigate to the device IP.
    - Validate critical paths: Settings page, Data reload, and Dashboard rendering.

## Resource Impact Assessment

### Memory (Flash/RAM/Stack/Heap)
- **Flash**: No significant change (pruning includes may slightly reduce binary size).
- **RAM/Heap**: **Significant Reduction Expected**. Lazy-loading of system boards should reduce initial heap allocation by ~15-20% by avoiding static instantiation of unused UI components.
- **Stack**: Multi-core FreeRTOS stack usage will be monitored via serial for any overflows in the new unified boot task.

### Power & Connectivity
- **Power**: No change to sleep modes.
- **Connectivity**: Validating that the NTP sync and data fetch logic (remapped to 25-100% progress) reliably establishes WiFi before reporting "Ready".

### Security
- **Attack Surface**: No new ports or services. Pruning entry point includes reduces potential side-channel information.

## Open Questions
- Is there a specific IP address or hostname I should expect for the device, or should I rely entirely on serial logs?

## Verification Plan

### Automated Tests
- `npm test` in `test/web` (Phase 1)
- `scripts/run_web_tests.py` (Phase 2)

### Manual Verification
- Visual inspection of the boot progress bar via serial.
- Interactive portal testing via the browser subagent.
