# Clock Display Review & Diagnostics

This plan refines the "Clock" board dialog by removing irrelevant fields, adding a per-board brightness override with a "Use Global" toggle, and implementing live NTP/time-sync diagnostics.

## UI Wireframe (Clock Display Editor)

```text
+------------------------------------------+
| Display Editor                   [Close] |
|                                          |
| Display Name                             |
| [ Clock / Screensaver                  ] |
|                                          |
| Board Brightness                         |
| [x] Use Global System Brightness         |
| [---------|---------o------------------] |
| 128 (Disabled if Global is checked)      |
|                                          |
| Diagnostics & Technical Details   [IDLE] v|
| +--------------------------------------+ |
| | NTP Status: Synchronized             | |
| | System Time: 18:55:14                | |
| +--------------------------------------+ |
|                                          |
| [ Test ]        [ Cancel ]      [ Apply ] |
+------------------------------------------+
```

## Proposed Changes

### Configuration & Backend

#### [MODIFY] [configManager.hpp](modules/configManager/configManager.hpp)
- Add `int brightness = 0` to `BoardConfig` struct (0 = Use Global).
- Add `MODE_CLOCK = 3` to `BoardTypes` enum.

#### [MODIFY] [configManager.cpp](modules/configManager/configManager.cpp)
- Update JSON serialization/deserialization to support per-board `brightness`.
- Update `validate()` to support `MODE_CLOCK`.

#### [MODIFY] [displayManager.cpp](modules/displayManager/displayManager.cpp)
- In `showBoard()`, if the board has a `brightness > 0`, apply it to the hardware. Otherwise, revert to the global system brightness.

### Web API & UI

#### [MODIFY] [webHandlerManager.cpp](modules/webServer/webHandlerManager.cpp)
- Update `handleTestBoard()` for "clock" type to return real NTP sync status and current system time.
- Sync `brightness` in `handleGetConfig` and `handleSaveAll`.

#### [MODIFY] [index.html](portal/index.html)
- **Add Brightness Controls**: Add a "Use Global" checkbox and a range input to the board editor modal.
- **Slider Logic**: Add an event listener to the checkbox to enable/disable the slider.
- **Toggle Visibility**: Hide "Filter" and show "Brightness" controls when board type is Clock.
- **Update Diagnostics**: Modify `renderBoardDiag` to show NTP/Time rows for Clock boards.

## Verification Plan

### Manual Verification
1.  Add a **Clock** display in the portal.
2.  Uncheck **Use Global System Brightness**.
3.  Set the **Board Brightness** to a low value (e.g., 20).
4.  Expand **Diagnostics**, click **Test**, and verify NTP status.
5.  **Apply** changes and verify the display dims when the Clock board is active.
6.  Re-edit, check **Use Global**, and verify the slider is disabled.
