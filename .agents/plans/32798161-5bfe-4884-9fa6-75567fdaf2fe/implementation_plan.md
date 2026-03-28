# Volatile Diagnostic Mode Configuration

Provide a way to activate the `DiagnosticBoard` from the Web Portal using a **run-time flag**. This mode will NOT be persisted in the configuration; it will automatically turn off if the device reboots or is reset.

## Proposed Changes

### [Firmware]

#### [MODIFY] [DisplayManager.hpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/displayManager.hpp)
- Add `bool diagModeActive = false;` to the `DisplayManager` class.
- Add `void setDiagMode(bool active);` and `bool getDiagMode() const;`.
- Add `DiagnosticBoard diagnosticBoard;` to the system boards registry (or handle via variant).

#### [MODIFY] [DisplayManager.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/displayManager.cpp)
- Implement `setDiagMode(bool active)` to trigger a `showBoard()` transition.
- Update `tick()` logic to ensure that if `diagModeActive` is true, the carousel rotation is paused and the diagnostic screen remains visible.

#### [MODIFY] [WebHandlerManager.hpp/cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/webServer/webHandlerManager.cpp)
- Add a new endpoint: `POST /api/system/diag?active=[true|false]`.
- This endpoint will directly call `displayManager.setDiagMode()`.

### [Web Portal]

#### [MODIFY] [index.html](file:///Users/mcneillm/Documents/Projects/departures-board/web/index.html)
- Add a "Show Calibration Grid" checkbox to the **System** tab.
- Add a JavaScript function `app.toggleDiagMode(checkbox)` that sends the POST request to the new API endpoint immediately upon change.
- Unlike other settings, this will **not** require clicking "Apply Settings" and will not be saved to the permanent `config.json`.

## Verification Plan

### Automated Tests
- `pio run`: Verify successful compilation.

### Manual Verification
1.  Open the Web Portal and navigate to the **System** tab.
2.  Toggle the **Show Calibration Grid** checkbox.
3.  Verify the physical display immediately shows the calibration grid.
4.  Reboot the device (via the Reboot button or power cycle).
5.  Verify the display returns to the standard carousel and the checkbox in the UI is unchecked.
