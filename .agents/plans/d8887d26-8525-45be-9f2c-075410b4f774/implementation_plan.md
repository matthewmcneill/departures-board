# Fix Mock Data Alignment and Enrich Layouts

The TfL and Bus boards in the layout simulator will be updated to include an "Order" column (1, 2, 3...) and consolidate fields to match real-world expectations.

## Proposed Changes

### 1. Layout Definitions (modules/displayManager/boards/*/layouts)

#### [MODIFY] [layoutDefault.json (TfL)](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/boards/tflBoard/layouts/layoutDefault.json)
Update columns and labels:
- Column 0: "Order" (width: 20, index 1, 2, 3...)
- Column 1: "Line" (width: 40)
- Column 2: "Destination" (width: 140)
- Column 3: "Time" (width: 56)

#### [MODIFY] [layoutDefault.json (Bus)](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/boards/busBoard/layouts/layoutDefault.json)
Update columns and labels:
- Column 0: "Order" (width: 20, index 1, 2, 3...)
- Column 1: "Route" (width: 25, route number)
- Column 2: "Destination" (width: 160)
- Column 3: "Time" (width: 51)

### 2. Mock Data (tools/layoutsim/mock_data)

#### [MODIFY] [tflBoard.json](file:///Users/mcneillm/Documents/Projects/departures-board/tools/layoutsim/mock_data/tflBoard.json)
Reorder and align the `services` array to [Order, Line, Destination, Time/Status, Platform]:
- Index 0: "1", "2", "3" (Order)
- Index 1: Line Name (e.g., "District")
- Index 2: Destination (e.g., "Richmond")
- Index 3: Time/Status (e.g., "Due", "1 min", "Approaching")
- Index 4: Platform (e.g., "1", "2")
- Index 5: (Empty string to satisfy 5-element parser minimum)

#### [MODIFY] [busBoard.json](file:///Users/mcneillm/Documents/Projects/departures-board/tools/layoutsim/mock_data/busBoard.json)
Reorder and align the `services` array to [Order, Route, Destination, Time/Status, Stop]:
- Index 0: "1", "2", "3" (Order)
- Index 1: Route (e.g., "341")
- Index 2: Destination (e.g., "Waterloo")
- Index 3: Time/Status (e.g., "Due", "1 min", "Approaching")
- Index 4: Stop (e.g., "Stop H")
- Index 5: (Empty string to satisfy 5-element parser minimum)

## Verification Plan
### 3. Data Sources (modules/displayManager/boards/*)

#### [MODIFY] [tflDataSource.hpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/boards/tflBoard/tflDataSource.hpp)
- Add `char platformName[TFL_MAX_LOCATION]` to the `TflService` struct.

#### [MODIFY] [tflDataSource.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/boards/tflBoard/tflDataSource.cpp)
- Parse the `platformName` field from the TfL API JSON into the `TflService` struct.

### 4. Board Controllers (modules/displayManager/boards/*)

#### [MODIFY] [tflBoard.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/boards/tflBoard/tflBoard.cpp)
Update `updateData()` logic for multidimensional uniqueness:
- Check if all services share the same **Line Name**.
- Check if all services share the same **Platform** (Direction).
- Hoist common values to the header (e.g., "Central Line, Platform 1").
- Populate `servicesWidget` with 4 columns:
  - Column 0: "Order" (1, 2, 3...)
  - Column 1: "Line" (Empty if line is hoisted, else Line Name)
  - Column 2: "Destination" (Optionally append platform if not hoisted)
  - Column 3: "Time"

#### [MODIFY] [busBoard.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/boards/busBoard/busBoard.cpp)
Update `updateData()` logic:
- Populate `servicesWidget` with 4 columns:
  - Column 0: "Order" (1, 2, 3...)
  - Column 1: "Route"
  - Column 2: "Destination"
  - Column 3: "Time"

### 5. Web Server (modules/webServer)

#### [MODIFY] [webHandlerManager.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/webServer/webHandlerManager.cpp)
Fix missing field persistence in `handleGetConfig` and `handleSaveAll`:
- Add `tflLineFilter`, `tflDirectionFilter`, and `showServiceOrdinals` to the JSON export and import logic.

## Verification Plan

### Automated Tests
- Run `python3 tools/layoutsim/scripts/gen_layout_cpp.py --input modules/displayManager/boards/tflBoard/layouts/layoutDefault.json`
- Run `python3 tools/layoutsim/scripts/gen_layout_cpp.py --input modules/displayManager/boards/busBoard/layouts/layoutDefault.json`

### Manual Verification
- Re-launch the layout simulator and verify TfL/Bus board layouts.
- Open the web portal and verify that TfL Line and Direction filters save correctly.
- Verify the header correctly reflects "hoisted" line/platform info when common.
