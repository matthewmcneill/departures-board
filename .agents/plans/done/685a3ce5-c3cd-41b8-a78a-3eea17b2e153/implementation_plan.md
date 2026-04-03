# Implementation Plan: Screensaver Clock Settings Enhancements

This plan details the changes for improving the Screensaver Clock configuration dialog, including adding per-board brightness, removing redundant fields, updating diagnostics, and styling sliders to match the brand.

## Proposed Changes

### Configuration Model (Backend/C++)

#### [MODIFY] [configManager.hpp](modules/configManager/configManager.hpp)
- Add `MODE_CLOCK = 3` to the `BoardTypes` enum to formally recognize the clock type.
- Add `int brightness = -1;` to the `BoardConfig` struct. A value of `-1` will signify using the global system brightness.

#### [MODIFY] [configManager.cpp](modules/configManager/configManager.cpp)
- **`save()`**: Include the per-board `brightness` field in the `boards` JSON array.
- **`loadConfig()`**: Read the per-board `brightness` field from the `boards` JSON array (default to `-1`).
- **`validate()`**: Add a `case MODE_CLOCK:` to the `switch (bc.type)` block to mark clocks as always complete.

---

### Web Portal (Frontend/Web)

#### [MODIFY] [index.html](web/index.html)
- **CSS**: 
    - Add a global style for `input[type="range"]` to use `accent-color: var(--primary);` ensuring all sliders (system and board) are orange.
- **HTML**:
    - **Board Modal (`modal-board`)**:
        - Add a range input for `brightness` (0-255).
        - Add a label for the brightness slider.
        - Add `<tr>` rows for "Local Time" and "Timezone" to the diagnostics table.
- **JavaScript (`app`)**:
    - **`handleBoardTypeChange`**: 
        - Explicitly hide the `board-filters-section` when `type === 3` (Clock).
        - Show/Hide the new brightness slider based on the board type.
        - Show/Hide the "Local Time" and "Timezone" diagnostics rows based on the board type.
        - Hide the Lat/Lon diagnostics rows for clocks.
    - **`editBoard`**:
        - Populate the new brightness slider from the `BoardConfig` object (default to -1/Global).
    - **`updateStatusUI`**:
        - Update the real-time diagnostics for clocks to show the current device time and timezone string.

---

## Verification Plan

### Automated Tests
- **Pio Build**: Run `pio run` to verify that the C++ changes compile correctly.
- **Playwright Tests**:
    - Create a new test file [clock_config.spec.ts](test/web/tests/clock_config.spec.ts) based on the existing `portal.spec.ts`.
    - **Verify**:
        - Selecting "Screensaver / Clock" correctly hides the "Filter" field.
        - The new "Brightness" slider appears and is functional.
        - The diagnostics section correctly shows "Local Time" and "Timezone" instead of Lat/Lon.
        - Saving the clock configuration actually sends the `brightness` field in the payload.

### Manual Verification
1. **Visual Audit**:
    - Open the portal and verify all range sliders (System and Boards) are now orange.
2. **Clock Configuration**:
    - Select a "Clock" board and verify that the "Filter" and "Weather" fields are hidden.
    - Set a custom brightness and verify it persists after applying and reloading (if the backend is operational).
3. **Diagnostics**:
    - Open the diagnostics drawer in the clock editor and verify the "Local Time" and "Timezone" fields are visible and populated.
