# Implementation Plan - Text Primitive Refactoring

[x] Reviewed by house-style-documentation - passed
[x] Reviewed by architectural-refactoring - passed
[x] Reviewed by embedded-systems - passed, stack usage monitored

Consolidate text drawing primitives into a single, versatile `drawText` function and expose it as a primitive in the layout JSON.

## User Review Required

> [!IMPORTANT]
> This refactor will replace `drawTruncatedText` and `centreText`. I will update all 5 system board modules (`messageBoard.cpp`, etc.) to use the new `drawText` function.

> [!NOTE]
> `drawText` will use a 256-byte stack buffer for truncation, consistent with existing primitives. On ESP32, this is within safe limits for the UI task.

## Proposed Changes

### [Display Manager]

#### [MODIFY] [drawingPrimitives.hpp](modules/displayManager/widgets/drawingPrimitives.hpp)
- Define `enum class TextAlign : uint8_t { LEFT = 0, CENTER = 1, RIGHT = 2 };`.
- Consolidate text primitives into a unified `drawText` function using a standard box model:
  `void drawText(U8G2& display, const char* text, int x, int y, int w = -1, int h = -1, TextAlign align = TextAlign::LEFT, bool truncate = false, const uint8_t* font = nullptr);`
- **Note**: `x, y` will represent the **top-left** of the text area (consistent with widgets), and the function will internally calculate the baseline using the font's ascent.
- Provide overloads for `const __FlashStringHelper*`.

#### [MODIFY] [serviceListWidget.hpp](modules/displayManager/widgets/serviceListWidget.hpp)
- Update `ColumnDef` to use the `TextAlign` enum for consistency.

#### [MODIFY] [drawingPrimitives.cpp](modules/displayManager/widgets/drawingPrimitives.cpp)
- Implement `drawText` with alignment and truncation logic.
- If `font` is provided (not `nullptr`), call `display.setFont(font)`.
- Use `TextAlign::CENTER` and `TextAlign::RIGHT` for alignment logic.
- Remove old `drawTruncatedText` and `centreText` implementations.

#### [MODIFY] System Boards
Update the following files to use `drawText`:
- [messageBoard.cpp](modules/displayManager/boards/systemBoard/messageBoard.cpp)
- [wizardBoard.cpp](modules/displayManager/boards/systemBoard/wizardBoard.cpp)
- [helpBoard.cpp](modules/displayManager/boards/systemBoard/helpBoard.cpp)
- [loadingBoard.cpp](modules/displayManager/boards/systemBoard/loadingBoard.cpp)
- [splashBoard.cpp](modules/displayManager/boards/systemBoard/splashBoard.cpp)

### [Tools]

#### [MODIFY] [gen_layout_cpp.py](tools/layoutsim/scripts/gen_layout_cpp.py)
- Update `primitives` parsing loop to support `type: "text"`.
- Support properties: `text`, `font` (optional), `align` ("LEFT", "CENTER", "RIGHT" - default to "LEFT"), `truncate` (boolean - default to `false`).
- Map JSON alignment strings to `TextAlign` enum values.

## Verification Plan

### Automated Tests
- Run `npm run build` (or `pio run`) to ensure no compilation errors after refactoring.
- Check the layout simulator output to ensure fixed-text primitives render correctly.

### Manual Verification
1. Update `layoutDefault.json` to include a static text primitive (e.g., a "DEPARTURES" header).
2. Run `python3 tools/layoutsim/scripts/gen_layout_cpp.py --input modules/displayManager/boards/nationalRailBoard/layouts/layoutDefault.json`.
3. Verify the generated `.cpp` file contains the correct `drawText` call in the `render()` method.
4. Open the layout simulator in a browser and verify the text appears as expected.
