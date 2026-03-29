# Implementation Plan - Fix Missing Layout Generation (System Board)

Update the automated layout generation script to include the `systemBoard` directory, ensuring `layoutDiagnostic.hpp` is correctly generated during the build.

## Proposed Changes

### Build Scripts

#### [MODIFY] [generate_layouts.py](file:///Users/mcneillm/Documents/Projects/departures-board/scripts/generate_layouts.py)
Add the `systemBoard` path to the automated layout generation loop.

```python
    search_paths = [
        "modules/displayManager/boards/nationalRailBoard/layouts/*.json",
        "modules/displayManager/boards/tflBoard/layouts/*.json",
        "modules/displayManager/boards/busBoard/layouts/*.json",
        "modules/displayManager/boards/systemBoard/layouts/*.json"  # <-- ADDED
    ]
```

## Open Questions

None.

## Verification Plan

### Automated Tests
1. **Claim Lock**: Run `/plan-start` to claim the hardware lock.
2. **Execute Build**: Run `pio run` and verify the output log for `Processing modules/displayManager/boards/systemBoard/layouts/layoutDiagnostic.json...`.
3. **Check Artifacts**: Confirm `modules/displayManager/boards/systemBoard/layouts/layoutDiagnostic.hpp` exists and has been updated.
