[x] Reviewed by architectural-refactoring - passed
[x] Reviewed by house-style-documentation - passed
[x] Reviewed by embedded-systems - passed

# Integration of Weather Widget into TfL and Bus Layouts

The weather widget is currently fully functional in the backend but is only utilized by the National Rail display board. This task will integrate the widget into the visual JSON layouts for both the Transport for London (TfL) and Bus display boards, achieving UI consistency across all active transit modes.

## User Review Required

No user review required for architectural concerns. A visual review in the layout simulator is recommended to ensure the location header text has sufficient width after squeezing in the new weather icon.

## Proposed Changes

### TfL Board Layout
Adjust the width of the `locationAndFilters` widget to make room for the new `weather` widget.

#### [MODIFY] layoutDefault.json(modules/displayManager/boards/tflBoard/layouts/layoutDefault.json)
Shrink `locationAndFilters` width from 236 to 225. Insert the `weatherWidget` definition natively alongside the `wifiWarning` widget in the top header row.

### Bus Board Layout
Adjust the width of the `locationAndFilters` widget to make room for the new `weather` widget.

#### [MODIFY] layoutDefault.json(modules/displayManager/boards/busBoard/layouts/layoutDefault.json)
Shrink `locationAndFilters` width from 236 to 225. Insert the `weatherWidget` definition natively alongside the `wifiWarning` widget in the top header row.

## Verification Plan

### Automated Tests
Run the WASM display simulator (`tools/layoutsim/scripts/dev_server.py`) with mock data payloads that include a `"weather"` block, verifying that the icon renders in the top right corner without overlapping the location text.

### Manual Verification
Run `pio run -e esp32dev -t upload && pio device monitor -e esp32dev` and visually confirm the weather icon appears successfully on the physical hardware when switched to TfL or Bus modes.
