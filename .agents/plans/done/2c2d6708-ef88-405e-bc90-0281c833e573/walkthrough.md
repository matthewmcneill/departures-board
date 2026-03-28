# Clock Widget Extension Walkthrough

## Summary
The system clock widget (`clockWidget`) has been refactored to support new layout parsing logic for the v3.0 platform, satisfying the need for flexible clock sizes and "seconds" digits that map to real public transport boards.

## Features Added
- **`ClockFormat` Expansion**: The clock now handles basic `HH_MM` as well as full `HH_MM_SS` formats.
- **Secondary Fonts**: The seconds portion of the digital clock can now be optionally constrained to a custom secondary font (e.g. `NatRailClockSmall7` alongside a standard `NatRailClockLarge9` primary font).
- **Baseline Alignment**: Mixed-font baselines are rigorously synchronized in C++ by evaluating U8g2 descent and ascent metrics.
- **Data Serialization**: The C++ and WASM Javascript layout parser systems have been fully modernized to hydrate `format` and `secondaryFont` tags directly from `.json` templates instead of relying on legacy hardcodes. 

## Testing Performed
- **Memory Diagnostics**: Confirmed that `lastSecond` variables don't bleed during board context switching.
- **WASM Hot-Reload**: Confirmed via the browser layout portal that the visual elements scale flawlessly together.
- **WASM UI Auto-Generation**: Updated the Javascript schema scanner so that clicking a `clockWidget` in the IDE injects a fully qualified `format` property configuration pane immediately.

The codebase now supports diverse variations in real-world clocks without requiring duplicate firmware variants.
