# Walkthrough: Intelligent Polling RDM

The dynamic polling feature for the National Rail RDM Data Provider has been successfully codified, compiled, and flashed to the hardware. 

## Changes Made
1. Transformed `modules/displayManager/boards/nationalRailBoard/nrRDMDataProvider.cpp` to integrate global clock availability via `appContext`.
2. Replaced the static `30000ms` fetch interval with a realtime wall-clock calculation:
   * Parses the string-based Scheduled (STD) or Estimated (ETD) Time of Departure for the next relevant train on the board.
   * Compares the localized train departure time against the active RTC value synced via NTP.
   * Dynamically tracks timezone offsets and resolves trailing midnight rollovers precisely using raw seconds logic.
   * Calculates a custom `delay = departure + 15` targeting an exact 15-second visual refreshment delay post-departure to optimize cache freshness.
   * Standardizes boundaries to enforce a **minimum delay of 15s** (preventing API thrashing if a train already left) and a **maximum delay of 60s** (ensuring board interactivity such as cancellations continues unhindered).

## Verification & Validation
* **Build Architecture**: Code natively compiled across both `esp32dev` and `esp32s3nano` core structures successfully.
* **Flashing**: Firmware binary securely loaded via the `/flash-test` testing workflow with no memory or watchdog issues detected during reboot.
* Logs confirm the device is heartbeat-verified in the background.

## Next Steps
1. Navigate via the Web UI and select National Rail on your local device to watch the `[DATA_RDM]` serial logs dynamically calculate the optimal next polling time!
2. You can use the `/plan-wrap` workflow to cleanly conclude this plan lifecycle constraint.
