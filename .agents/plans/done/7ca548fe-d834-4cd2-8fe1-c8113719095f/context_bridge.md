# Context Bridge

## 📍 Current State & Focus
The implementation of the `trainFormationWidget` has been fully completed. The C++ widget architecture natively extracts carriage layout geometries, calculates automatic spacing or marquee scrolling states, and uses pixel-perfect injects for National Rail fonts. The feature has been finalized, validated via the ESP-32 UART pipeline against real RDM JSON payloads, and verified crash-safe under missing telemetry edge cases.

## 🎯 Next Immediate Actions
None. This plan cycle is formally marked `DONE`. A new plan was spun up specifically targeting the upstream resolution of API formation JSON keys.

## 🧠 Decisions & Designs
- Used `ArduinoJson` to natively drop/filter unneeded formations payload branches efficiently on memory-constrained hardware.
- Allowed the widget to safely abort `draw()` if `coaches` array is missing, preventing segmentation faults.
- Added detailed C++ module headers enforcing Gadec house style documentation.

## 🐛 Active Quirks, Bugs & Discoveries
- The live LDBWS JSON endpoint natively strips the `formation` key entirely.
- The SV (Staff Version) LDBSVWS endpoint does supply the `formation` container, but specific test services (e.g. Ewell West and London Waterloo tonight) frequently return empty `serviceLoading` objects missing the inner `coaches` array dynamically.

## 💻 Commands Reference
- `pio test -e unit_testing_host` (Currently broken due to `dataManager` forward declaration dependencies; ticket opened)
- `./tools/safe-flash.sh`

## 🌿 Execution Environment
Hardware ESP32 flashed. Mock ESP32 test workflows and the native suite utilized for layout visualization.
