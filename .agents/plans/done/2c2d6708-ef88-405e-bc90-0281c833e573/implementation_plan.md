# Extend Clock Widget Formats

[x] Reviewed by house-style-documentation - passed
[x] Reviewed by architectural-refactoring - passed
[x] Reviewed by embedded-systems - passed: minimal impact (stack/flash)
[x] Reviewed by embedded-web-designer - ASCII mockups included

Extend the `clockWidget` to support `HH:MM:SS` and other common departure board formats as identified from user-provided images.

## User Review Required

### Clock Variants Mockups

**1. Standard (HH:MM)**
```text
+-----------------------+
|         14:36         |
+-----------------------+
```

**2. Full (HH:MM:SS)**
```text
+-----------------------+
|       14:36:50        |
+-----------------------+
```

## Font and Size Analysis

## Font and Size Analysis

Based on a closer re-examination of the provided images:

1. **Size Variations**:
   - In most variants, the seconds are the same height as the hours and minutes.
   - However, in at least one National Rail board style (Image 2), the **seconds are rendered using a smaller font** than the hours and minutes (e.g., `14:36:00` where `00` is smaller).
2. **Font Characteristics**:
   - **TfL/Bus**: Uses a bold dot-matrix font with rounded dot shapes (e.g., `UndergroundClock8`).
   - **National Rail**: Uses a high-contrast LED font with vertically elongated characters (e.g., `NatRailClockLarge9` and `NatRailClockSmall7`).
3. **Alignment**: When smaller seconds are used, the text appears to be roughly vertically centered or slightly bottom-aligned relative to the larger text, with the separator colons often centered.

### Proposed Font Strategy

- I will implement an enum `ClockFormat` with two options:
  - `HH_MM` (Default, standard)
  - `HH_MM_SS` (With seconds)
- Size control is decoupled from the format:
  - If `format` is `HH_MM_SS` and no secondary font is provided, the primary font is used for all digits.
  - If `secondaryFont` is provided, the widget will render the seconds using this smaller font, baseline-aligned with the primary font.
- Calculate accurate widths based on the fonts used to ensure correct center alignment of the widget as a whole.

## Proposed Changes

### Display Manager Widgets

#### [MODIFY] [clockWidget.hpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/widgets/clockWidget.hpp)

- Add `enum class ClockFormat { HH_MM, HH_MM_SS };`
- Add `ClockFormat format;` member variable.
- Add `const uint8_t* secondaryFont;` member variable.
- Add `setFormat(ClockFormat newFormat)` method.
- Add `setSecondaryFont(const uint8_t* newFont)` method.
- Add `@designer_prop` annotations for the new format and secondary font properties to expose them to the UI/Layout JSON.

#### [MODIFY] [clockWidget.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/widgets/clockWidget.cpp)

- Update constructor to initialize `format` to `HH_MM` and `secondaryFont` to `nullptr`.
- Update `render()` to handle the two formats:
  - Parse the time components (hours, minutes, seconds).
  - Calculate `totalW` dynamically based on the active format. Use `display.setFont()` to measure widths with the correct font for each string component.
  - Draw the components sequentially, switching to `secondaryFont` for the seconds if it has been configured.
  - **Baseline Alignment**: Since v3.0 uses `setFontPosTop()` globally, calculate the baseline of the primary font (`mainBaselineY = drawY + display.getAscent()`). If a secondary font is used, calculate its top Y coordinate relative to the main baseline (`smallDrawY = mainBaselineY - display.getAscent()`) using the metrics of the secondary font.
- Update `renderAnimationUpdate()` to trigger a partial redraw if the format includes seconds and the seconds value changes.

#### [MODIFY] [layoutDefault.json (National Rail)](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/boards/nationalRailBoard/layouts/layoutDefault.json)

- Update the `sysClock` widget configuration to test the new format.
- Add `"format": "HH_MM_SS"` and `"secondaryFont": "NatRailClockSmall7"`.

---

## Verification Plan

### Automated Tests
- I will use the WASM Layout Simulator to verify the rendering of the new formats across different boards.
- I will create a temporary test layout that uses the `clockWidget` with `HH:MM:SS`.

### Manual Verification
1. **Simulator Test**: Open the WASM simulator and verify that the clock renders correctly with seconds.
2. **Visual Audit**: Compare the simulator output with the provided real-world images to ensure font and formatting accuracy.
3. **Blink Test**: Verify that the colon(s) blink as expected (1Hz).
