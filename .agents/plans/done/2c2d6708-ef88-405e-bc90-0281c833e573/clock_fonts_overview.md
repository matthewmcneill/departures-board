# Clock Fonts Overview

Here is an overview of the fonts currently available in the system for rendering clock digits, along with recommendations for how to combine them for the different formats.

## Available Fonts

Based on `modules/displayManager/fonts/fonts.hpp`, we have the following fonts specifically intended for clocks or often used for them:

| Font Name | Style / Usage | Characteristics |
| :--- | :--- | :--- |
| `NatRailClockLarge9` | High-contrast LED (National Rail) | Vertically elongated, thinner stroke width. Best for primary `HH:MM` display on National Rail boards. |
| `NatRailClockSmall7` | Small LED (National Rail) | A scaled-down version of the Large LED font. Perfect for the "small seconds" or for tighter vertical spaces. |
| `NatRailTall12` | General Purpose National Rail | Often used for service times in rows, but could theoretically be used for a clock if a heavier LED look is desired. |
| `UndergroundClock8` | Dot-matrix (TfL / Bus / SWR style) | Bold, square/rounded dots, wider characters. Highly visible. |

## Recommended Combinations

When configuring the layout JSON for the new formats, here are the recommended combinations based on the real-world boards:

### 1. National Rail (Small Seconds)

This is the classic National Rail look where the seconds are tucked in slightly smaller.

*   **Format**: `HH_MM_ss`
*   **Primary Font (`font`)**: `NatRailClockLarge9` (for the HH and MM)
*   **Secondary Font (`smallFont`)**: `NatRailClockSmall7` (for the ss)

### 2. TfL / London Underground (Full Size)

TfL boards universally use a bold dot-matrix font where the seconds are the same size as the hours and minutes.

*   **Format**: `HH_MM_SS`
*   **Primary Font (`font`)**: `UndergroundClock8` (for HH, MM, and SS)
*   **Secondary Font**: Not required.

### 3. South Western Railway (SWR) / Heavy Dot Matrix

Similar to TfL, these modern full-color or high-density orange LED boards use a very bold font where all digits are the same size.

*   **Format**: `HH_MM_SS`
*   **Primary Font (`font`)**: `UndergroundClock8` (or potentially a new `SWRClock10` if we add one later).
*   **Secondary Font**: Not required.

### Layout JSON Configuration Example

To implement the National Rail style with small seconds in your layout JSON, the `clockWidget` configuration will look like this:

```json
{
  "id": "sysClock",
  "type": "clockWidget",
  "geometry": {
    "x": 216,
    "y": 0,
    "w": 40,
    "h": 11
  },
  "visible": true,
  "format": "HH_MM_ss",
  "font": "NatRailClockLarge9",
  "smallFont": "NatRailClockSmall7"
}
```
