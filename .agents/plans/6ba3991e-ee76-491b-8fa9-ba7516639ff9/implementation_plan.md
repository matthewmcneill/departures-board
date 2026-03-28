# WiFi Signal Icon Design (11x11)

The goal is to create a set of 5 icons (None, 1 bar, 2 bars, 3 bars, 4 bars) constrained to an 11x11 pixel grid for the departures board header.

## User Review Required

Please review the following ASCII mockups for the 11x11 icons. `.` represents an empty pixel, `#` represents a lit pixel.

### [Option 1: Classic Curved Arcs]

This design fits the traditional WiFi shape into the tight 11x11 grid.

**Strength 4 (Full)**
```
. . . # # # # # . . .
. . # . . . . . # . .
. # . . # # # . . # .
# . . # . . . # . . #
. . # . . # . . # . .
. . . . # . # . . . .
. . . . . # . . . . .
. . . . . . . . . . .
. . . . . . . . . . .
. . . . . . . . . . .
. . . . . . . . . . .
```

Wait, 11x11 is very small for 4 arcs. Let's try a simpler "wedge" style or a cleaner arc.

**Strength 4 (Wedge Style - more visible)**
```
. . # # # # # # # . .
. # # # # # # # # # .
# # . . . . . . . # #
. . # # # # # # # . .
. # # . . . . . # # .
. . . # # # # # . . .
. . . # . . . # . . .
. . . . # # # . . . .
. . . . . # . . . . .
. . . . . . . . . . .
. . . . . . . . . . .
```

Actually, let's try to match the user's PNGs more closely. The PNGs show a triangular/fan shape.

### [Option 2: High-Visibility Fan (Recommended)]

**Strength 4 (Full)**
```
. # # # # # # # # # .
# # # # # # # # # # #
. . . . . . . . . . .
. . # # # # # # # . .
. . # # # # # # # . .
. . . . . . . . . . .
. . . . # # # . . . .
. . . . # # # . . . .
. . . . . . . . . . .
. . . . . # . . . . .
. . . . . # . . . . .
```

**Strength 0 (None with X)**
```
. # . . . . . . . # .
# . # . . . . . # . #
. # . . . . . . . # .
. . . . . . . . . . .
. . . . . . . . . . .
. . . . . . . . . . .
. . . . . . . . . . .
. . . . . . . . . . .
. . . . . . . . . . .
. . . . . # . . . . .
. . . . . # . . . . .
```
*(Only showing top arc and dot for 'None' state, with an X at the top?)*

Let's do a better set.

| Strength | Mockup (11x11 Grid) |
| :--- | :--- |
| **None (X)** | <pre>  . . . . . # . . . . .<br>  . . . . . # . . . . .<br>  . . . . . . . . . . .<br>  . . # . . . . . # . .<br>  . . . # . . . # . . .<br>  . . . . # . # . . . .<br>  . . . . . # . . . . .<br>  . . . . # . # . . . .<br>  . . . # . . . # . . .<br>  . . # . . . . . # . .<br>  . . . . . . . . . . .</pre> |
| **1 Bar** | <pre>  . . . . . . . . . . .<br>  . . . . . . . . . . .<br>  . . . . . . . . . . .<br>  . . . . . . . . . . .<br>  . . . . . . . . . . .<br>  . . . . . . . . . . .<br>  . . . . . . . . . . .<br>  . . . . . . . . . . .<br>  . . . . . # . . . . .<br>  . . . . # # # . . . .<br>  . . . . . # . . . . .</pre> |
| **2 Bars** | <pre>  . . . . . . . . . . .<br>  . . . . . . . . . . .<br>  . . . . . . . . . . .<br>  . . . . . . . . . . .<br>  . . . # # # # # . . .<br>  . . # # # # # # # . .<br>  . . . . . . . . . . .<br>  . . . . . . . . . . .<br>  . . . . . # . . . . .<br>  . . . . # # # . . . .<br>  . . . . . # . . . . .</pre> |
| **3 Bars** | <pre>  . . . . . . . . . . .<br>  . . # # # # # # # . .<br>  . # # # # # # # # # .<br>  . . . . . . . . . . .<br>  . . . # # # # # . . .<br>  . . # # # # # # # . .<br>  . . . . . . . . . . .<br>  . . . . . . . . . . .<br>  . . . . . # . . . . .<br>  . . . . # # # . . . .<br>  . . . . . # . . . . .</pre> |
| **4 Bars** | <pre>  # # # # # # # # # # #<br>  # # # # # # # # # # #<br>  . . . . . . . . . . .<br>  . . # # # # # # # . .<br>  . . # # # # # # # . .<br>  . . . . . . . . . . .<br>  . . . # # # # # . . .<br>  . . . # # # # # . . .<br>  . . . . . . . . . . .<br>  . . . . . # . . . . .<br>  . . . . . # . . . . .</pre> |

> [!NOTE]
> Based on the PNGs, the icons use **filled blocks** rather than single-pixel arcs. The 11x11 grid is very tight, so I've used 2-pixel high blocks for the larger arcs to give them some "weight" as seen in the source images.

### [Refined 11x11 Set (Filled)]

**4 Bars (Full - Direct PNG Translation)**
```
00  # # # # # # # # # # #
01  # # # # # # # # # # #
02  . . . . . . . . . . .
03  . . # # # # # # # . .
04  . . # # # # # # # . .
05  . . . . . . . . . . .
06  . . . # # # # # . . .
07  . . . # # # # # . . .
08  . . . . . . . . . . .
09  . . . . . # . . . . .
10  . . . . . # . . . . .
```

**None (Disconnected - Direct PNG Translation)**
```
00  . . . . . # . . . . .
01  . . . . . # . . . . .
02  . . . . . . . . . . .
03  . . # . . . . . # . .
04  . . . # . . . # . . .
05  . . . . # . # . . . .
06  . . . . . # . . . . .
07  . . . . # . # . . . .
08  . . . # . . . # . . .
09  . . # . . . . . # . .
10  . . . . . . . . . . .
```
*(The 'None' state uses a vertical tick at the top and a large centered 'X' as per the PNG concept.)*

## Proposed Changes

### [Display Components]

#### [MODIFY] [wifiStatusWidget.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/widgets/wifiStatusWidget.cpp)
Update to fetch RSSI from `DataManager` and render the appropriate icon state.

#### [NEW] [wifiIcons.hpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/fonts/wifiIcons.hpp)
Define the 11x11 bitmaps for the 5 states.

## Verification Plan

### Automated Tests
- WASM Simulator verification of all 5 states.

### Manual Verification
- Visual check on real OLED hardware.
