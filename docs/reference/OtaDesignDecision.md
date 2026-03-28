# OTA Update Strategy & Design Decision

## Context
During the v3.0 refactor, an `otaStatusWidget` (an up-arrow icon) was included in the display layouts. The original intention was that the system would silently poll for firmware updates and, if one was available, illuminate the widget to notify the user. The user could then trigger the download and restart at their convenience via the Web UI (the "Mobile OS" approach).

However, the existing `otaUpdater` logic was implemented as a disruptive, forced update mechanism (the "Appliance" approach). When a daily check finds a new release, it immediately commandeers the screen with a 30-second countdown (via `FirmwareUpdateBoard`), downloads the binary to the OTA partition, and restarts the ESP32. 

Because the update is forced immediately upon detection, there is no "pending" state for the `otaStatusWidget` to display. The widget was therefore functionally useless and the `otaUpdateAvailable` flag was being accidentally misused as a momentary "busy" indicator during unrelated background API fetches.

## Decision
On 2026-03-27, it was decided to stick with the **"Appliance" Approach**.
- The `otaStatusWidget` UI components were deleted from the firmware and simulator to save flash/RAM and simplify the layouts.
- The `otaUpdateAvailable` flag was removed from `systemManager` and `DisplayManager`.
- The forced, overnight, zero-touch update logic remains intact.

## Future Considerations
If we ever decide to change the update model to require user consent (the "Mobile OS" approach) to prevent disruptive reboots during peak viewing hours, the following work would need to be reinstated:
1. Re-add `otaStatusWidget` to `modules/displayManager/widgets`.
2. Add `otaStatusWidget` into the `i<Board>Layout.hpp` interfaces and the corresponding `layoutDefault.json` layouts.
3. Modify `otaUpdater::tick()` to perform a silent check and set an `updateAvailable` flag instead of triggering `FirmwareUpdateBoard`.
4. Add a "Trigger Update" button to the Web UI that manually invokes the download and reboot sequence.
