# Display Layouts Reference

This document provides a comprehensive guide to understanding, creating, and previewing display layouts in the Departures Board project.

## Overview
Layouts define the visual arrangement of widgets (clocks, labels, service lists) and drawing primitives (lines, boxes, text) on the physical OLED display.

Currently, the project uses a hybrid approach:
1. **JSON Layouts**: Used as the source of truth for the **Layout Simulator**.
2. **C++ Layouts**: The actual code compiled for the ESP32 hardware (e.g., `layoutDefault.cpp`).

**Future Roadmap**: The JSON layout definitions will eventually become the sole source of truth, from which the C++ header and source files will be automatically generated.

## The Layout Simulator (`layoutsim`)
The Layout Simulator is a powerful, live-reloading tool that allows you to preview and debug JSON layout changes in real-time within your browser, using the exact same C++ rendering engine (compiled to WebAssembly) as the physical board.

### How to Run the Simulator
1. Open a terminal and navigate to the repository root.
2. Start the development server:
   ```bash
   python3 tools/layoutsim/scripts/dev_server.py
   ```
3. Open the provided link in your browser: `http://localhost:8000/tools/layoutsim/web/index.html`

### Rebuilding the Widget Registry
The Layout Simulator is inextricably linked to the physical C++ headers of your displays (e.g. `iNationalRailLayout.hpp`). If you add or rename a physical `iGfxWidget` variable in your C++ interface, you must rebuild the simulator so it can dynamically extract the updated "Element Explorer" widget registry from the C++ AST.

Run the build script to regenerate the registry (`gen_sim_registry.py`) and compile the new WASM engine:
```bash
python3 tools/layoutsim/scripts/build_wasm.py
```
*(Requires Emscripten to be installed in the `/tools/layoutsim/emsdk` directory)*

### Using the Simulator
For the best ergonomic experience, snap your browser window to one side of your screen and your code editor (e.g., VS Code) to the other.

![Layout Simulator Interface](/Users/mcneillm/.gemini/antigravity/brain/d84b7bd1-ebd1-40f4-b15b-db00f504074e/layout_sim_updated_ui_1774291667968.png)
*(Example: The Layout Simulator running alongside the code editor.)*

- **Smart "Auto-detect" Mode**: By default, the simulator will automatically track your file changes. If you start editing `tflBoard/layouts/layoutDefault.json`, the simulator will instantly detect the change and switch its preview to match.
- **Persistent Element Explorer**: The sidebar always displays a complete list of every widget and primitive in the current layout.
- **Visual Bounding Boxes**: Click "Show Boxes" in the Explorer to overlay precise bounding box grids on the OLED display to verify element positioning and dimensions.
- **Live Inspection**: Click any item in the Explorer or on the display to view its exact JSON properties and geometry in the Property Inspector panel.
- **Real-time Diagnostics**: If you make a JSON syntax error, the sidebar will instantly turn red with precise error feedback, while the OLED safely displays your "Last Known Good" layout.

---

## JSON Format Reference
Every layout JSON file consists of two top-level arrays: `widgets` and `primitives`.

```json
{
  "layout": "iNationalRailLayout",
  "widgets": [
    {
      "id": "clock",
      "type": "clockWidget",
      "geometry": {
        "x": 200,
        "y": 0,
        "w": 56,
        "h": 12
      },
      "font": "NatRailClockSmall7",
      "blink": true
    }
  ],
  "primitives": [
    {
      "type": "line",
      "geometry": {
        "x1": 0,
        "y1": 13,
        "x2": 255,
        "y2": 13
      }
    }
  ]
}
```

---

## Widget Reference
Widgets are stateful UI components implemented in C++ (inheriting from `iGfxWidget`). Unlike primitives, widgets encapsulate their own data-handling (e.g., a clock managing time, a service list managing trains).

You **must** provide the correct `id` for a widget as defined by the board's `DesignerRegistry` in C++ for it to receive styles from the JSON.

### Universal Widget Properties
These parameters can be applied to **any** widget:
- `id` (String, Required): The unique identifier corresponding to the C++ object (e.g., `headWidget`, `sysClock`, `servicesWidget`).
- `type` (String, Required): The exact C++ derived class of this widget. Stating this class powers precise IDE autocomplete.
    - Valid types include: `"headerWidget"`, `"serviceListWidget"`, `"scrollingTextWidget"`, `"clockWidget"`, `"labelWidget"`, `"scrollingMessagePoolWidget"`.
- `geometry` (Object, Required): Defines the size and position of the widget.
  - `x` (Int): X-coordinate of the top-left corner.
  - `y` (Int): Y-coordinate of the top-left corner.
  - `w` (Int, Optional): Width of the widget area.
  - `h` (Int, Optional): Height of the widget area.
- `visible` (Boolean, Default `true`): Whether the widget should be drawn.

### Specialized Properties
Certain properties are only consumed by specific widget types during parsing. If you use the `.json` schema properly, VS Code will only suggest these fields if the `type` property is set correctly:

- **`font` (String)**
  - Applicable to: `labelWidget`, `clockWidget`, `scrollingTextWidget`, `scrollingMessagePoolWidget`
  - Values: `NatRailSmall9`, `NatRailTall12`, `NatRailClockSmall7`, `NatRailClockLarge9`, `Underground10`, `UndergroundClock8`.
- **`text` (String)**
  - Applicable to: `labelWidget`
  - Description: Hardcodes the text content.
- **`blink` (Boolean)**
  - Applicable to: `clockWidget`
  - Description: Toggles the blinking colon animation.
- **`columns` (Array of Objects)**
  - Applicable to: `serviceListWidget`
  - Description: Defines the width and text alignment of each column. Max 6 columns.
  - Example: `"columns": [ { "width": 25, "align": 0 }, { "width": 100, "align": 1 } ]` (Align: 0=Left, 1=Center, 2=Right).

---

## Drawing Primitive Reference
Primitives are static, stateless graphical marks drawn directly to the display by the layout engine. They are useful for static UI framing (lines, boundaries).

All primitives require a `geometry` object, but the fields inside it depend on the `type`.

### `line`
Draws a 1-pixel wide line between two points.
- `type`: `"line"`
- `geometry` (Object):
  - `x1` (Int): Starting X coordinate.
  - `y1` (Int): Starting Y coordinate.
  - `x2` (Int): Ending X coordinate.
  - `y2` (Int): Ending Y coordinate.

### `box`
Draws a filled rectangle.
- `type`: `"box"`
- `geometry` (Object):
  - `x` (Int): X-coordinate of the top-left corner.
  - `y` (Int): Y-coordinate of the top-left corner.
  - `w` (Int): Width.
  - `h` (Int): Height.

### `text`
Draws a static string of text.
- `type`: `"text"`
- `geometry` (Object):
  - `x` (Int): X-coordinate (baseline aligned).
  - `y` (Int): Y-coordinate (baseline aligned).
- `text` (String): The text to display.
- `font` (String): The font identifier (see Widget `font` property for valid options).

---
*Note: The coordinate system has its origin (0,0) at the top-left of the display.*
