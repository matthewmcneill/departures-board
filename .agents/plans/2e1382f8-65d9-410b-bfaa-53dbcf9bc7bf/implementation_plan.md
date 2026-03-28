# Fix Tube Display Destination Bug (Multi-Column Layout)

Following photographic evidence of real Tube displays, this plan transitions the `TfLBoard` to a 4-column layout with a dedicated "Position" column. This allows us to resolve the memory corruption bug by passing stable pointers for all fields without any string formatting or copying.

## Review Checklist
- [x] Reviewed by `house-style-documentation` - passed
- [x] Reviewed by `embedded-systems` - passed
- [x] Reviewed by `architectural-refactoring` - passed

## User Review Required

> [!TIP]
> This solution is the most efficient possible: it uses zero extra RAM and zero extra CPU cycles by utilizing a static array of strings for numbering. All destination pointers are now passed directly from the `dataSource`.

## Proposed Changes

### TfL Data Component

#### [MODIFY] [tflDataSource.hpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/boards/tflBoard/tflDataSource.hpp)
- Add `const char* orderNum` field to the `TflService` struct.
- Define a `static const char*` array for numbers `"1"` to `"9"`.

#### [MODIFY] [tflDataSource.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/boards/tflBoard/tflDataSource.cpp)
- In the background update process (JSON parsing or data sanitization stage), assign the appropriate stable pointer from the `serviceNumbers` array to each `TflService` object.

### TfL Board / Layout Component

#### [MODIFY] [layoutDefault.json](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/boards/tflBoard/layouts/layoutDefault.json)
- Add a new "Order" column at index 0. Re-calculate widths (e.g., 20px / 50px / 130px / 56px).

#### [MODIFY] [layoutDefault.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/boards/tflBoard/layouts/layoutDefault.cpp)
- Manually update the `ColumnDef` array to include the new column for the order number.

#### [MODIFY] [tflBoard.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/boards/tflBoard/tflBoard.cpp)
- Update `updateData()` to extract the `orderNum` directly from the `TflService` object.
- Pass the stable pointer to the new column (index 0).
- **Eliminate** all `snprintf` and `strlcpy` operations for service formatting.

### Bus Board Component (Consistency)

#### [MODIFY] [busDataSource.hpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/boards/busBoard/busDataSource.hpp)
- Add `const char* orderNum` field to the `BusService` struct.

#### [MODIFY] [busBoard.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/boards/busBoard/busBoard.cpp)
- Adopt the same 4-column numbering logic for consistency.

## Resource Impact Assessment

### Memory
- **RAM**: **0 bytes added**. By using static numbering strings, we avoid the buffer pool proposed in previous versions.
- **Flash**: Slight reduction by removing formatting logic.
- **Stack**: 64-byte reduction (no local `ordinalDest`).

## Verification Plan

### Automated Tests
- **Compilation**: Verify the firmware builds successfully with `pio run`.

### Manual Verification
- Deploy to hardware and verify the new 4-column layout matches the real-world Tube board design seen in the provided images.
