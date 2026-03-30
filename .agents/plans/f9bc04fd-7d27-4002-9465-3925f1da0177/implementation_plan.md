# Restore Simulator Display Rendering

The Layout Simulator is currently showing a black display because it is inadvertently using an empty "dummy" mock for the U8g2 library instead of the real library logic. This shadowing occurred because `test/mocks/` was given priority in the include paths.

## User Review Required

> [!IMPORTANT]
> I will be reconfiguring the WASM build to link the real U8g2 library. This will increase the WASM bundle size slightly but is necessary for pixel-accurate rendering.

## Proposed Changes

### [Simulator Engine]

#### [MODIFY] [build_wasm.py](file:///Users/mcneillm/Documents/Projects/departures-board/tools/layoutsim/scripts/build_wasm.py)
- Reorder include flags to prioritize `U8G2_LIB_DIR` over `test/mocks`.
- This ensures the real U8g2 library is linked instead of the empty dummy mock.

#### [MODIFY] [main.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/tools/layoutsim/src/main.cpp)
- Remove any local shadow defines (like `U8G2_R0`) that conflict with the real library.

### [Simulator Web UI]

#### [MODIFY] [index.html](file:///Users/mcneillm/Documents/Projects/departures-board/tools/layoutsim/web/index.html)
- Move the **Logic Injection** panel to the far left of the `.designer-panels` container.
- Ensure the **Element Explorer** remains in the middle and **Properties** on the far right.

## Verification Plan

### Automated Tests
- Run `python3 tools/layoutsim/scripts/build_wasm.py`.
- Verify no compilation errors are thrown now that the real (and much larger) U8g2 class is in play.

### Manual Verification
- Launch the simulator and verify that the canvas is no longer black.
- Use the "Logic Injection" panel to confirm that changing labels or icons reflects in the rendered pixels.
