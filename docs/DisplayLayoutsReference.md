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

## 1. JSON Document Structure
Every layout JSON file consists of a root object that must define the target C++ interface and provide arrays mapping to widgets and background primitives.

| Property | Type | Required | Default | Description |
| -------- | ---- | -------- | ------- | ----------- |
| `layout` | Enum (`String`) | Yes | - | The exact C++ interface class this layout maps to (e.g., `iNationalRailLayout`, `iTflLayout`, `iBusLayout`). |
| `widgets` | Array of Objects | No | `[]` | Stateful UI components managed by the C++ engine. |
| `primitives`| Array of Objects | No | `[]` | Stateless graphics drawn immediately during the layout rendering cycle. |

---

## 2. Shared Enums & Types
Many widgets and primitives share predefined values.

### Fonts
The `font` property relies on exactly matching strings that align with embedded C++ font assets:
- `NatRailSmall9`
- `NatRailTall12`
- `NatRailClockSmall7`
- `NatRailClockLarge9`
- `Underground10`
- `UndergroundClock8`

### Text Alignment
Used in components like `serviceListWidget` for column alignment:
- `0`: Left
- `1`: Center
- `2`: Right

---

## 3. Widgets API Reference
Widgets are stateful UI components implemented in C++ (inheriting from `iGfxWidget`). Unlike primitives, widgets encapsulate their own data-handling (e.g., a clock managing time, a service list managing trains). 

### Base Widget Properties
The following properties can be applied to **any** widget defined in the `widgets` array.

| Property | Type | Required | Default | Description |
| -------- | ---- | -------- | ------- | ----------- |
| `id` | String | Yes | - | The unique identifier corresponding to the C++ object variable name (e.g., `headWidget`, `sysClock`, `servicesWidget`). Cannot be arbitrary. |
| `type` | Enum (`String`) | Yes | - | The exact C++ derived class of this widget. |
| `geometry` | Object | Yes (for `x`, `y`)| - | Defines the position (`x`, `y`) and size (`w`, `h`) boundaries of the widget. |
| `visible` | Boolean | No | `true` | Determines if the widget should be drawn. |

### `headerWidget`
A standard header bar containing time and connection statuses.
- **Additional Properties:** None.
```json
{
  "id": "header",
  "type": "headerWidget",
  "geometry": { "x": 0, "y": 0, "w": 256, "h": 12 }
}
```

### `clockWidget`
A standalone clock element.
- **Additional Properties:**

| Property | Type | Required | Default | Description |
| -------- | ---- | -------- | ------- | ----------- |
| `font` | String | Yes | - | The font identifier string to use. |
| `blink` | Boolean | No | `false`| Enables the blinking colon animation. |
```json
{
  "id": "clock",
  "type": "clockWidget",
  "geometry": { "x": 200, "y": 0, "w": 56, "h": 12 },
  "font": "NatRailClockSmall7",
  "blink": true
}
```

### `labelWidget`
A static or system-driven text label.
- **Additional Properties:**

| Property | Type | Required | Default | Description |
| -------- | ---- | -------- | ------- | ----------- |
| `font` | String | Yes | - | The font identifier string to use. |
| `text` | String | No | - | Hardcodes the literal text content. |
```json
{
  "id": "noDataLabel",
  "type": "labelWidget",
  "geometry": { "x": 0, "y": 20 },
  "font": "NatRailSmall9",
  "text": "Please Wait..."
}
```

### `scrollingTextWidget`
A marquee-style scrolling text element.
- **Additional Properties:**

| Property | Type | Required | Default | Description |
| -------- | ---- | -------- | ------- | ----------- |
| `font` | String | Yes | - | The font identifier string to use. |
```json
{
  "id": "scrollMsg",
  "type": "scrollingTextWidget",
  "geometry": { "x": 0, "y": 64, "w": 256, "h": 12 },
  "font": "NatRailSmall9"
}
```

### `scrollingMessagePoolWidget`
A specialized scroller managing multiple sequential messages.
- **Additional Properties:**

| Property | Type | Required | Default | Description |
| -------- | ---- | -------- | ------- | ----------- |
| `font` | String | Yes | - | The font identifier string to use. |
```json
{
  "id": "messages",
  "type": "scrollingMessagePoolWidget",
  "geometry": { "x": 0, "y": 50, "w": 256, "h": 14 },
  "font": "NatRailSmall9"
}
```

### `serviceListWidget`
A list managing rows and columns mapping to API train/bus services data.
- **Additional Properties:**

| Property | Type | Required | Default | Description |
| -------- | ---- | -------- | ------- | ----------- |
| `skipRows` | Integer | No | `0` | Number of rows to discard from the beginning of the data set. |
| `maxRows` | Integer | No | `-1` | Maximum number of rows to ingest limit (`-1` for unbounded). |
| `columns` | Array | Yes | - | An array of column definition objects (max 6 items). |

**Column Definition Object:**
| Property | Type | Required | Default | Description |
| -------- | ---- | -------- | ------- | ----------- |
| `width` | Integer | Yes | - | Column width in pixels. |
| `align` | Integer | Yes | - | Text alignment (`0`=Left, `1`=Center, `2`=Right). |
| `label` | String | No | - | Optional column header label. |

```json
{
  "id": "services",
  "type": "serviceListWidget",
  "geometry": { "x": 0, "y": 14 },
  "skipRows": 0,
  "maxRows": 2,
  "columns": [
    { "width": 25, "align": 0 },
    { "width": 100, "align": 1 }
  ]
}
```

---

## 4. Primitives API Reference
Primitives are static, stateless graphical marks drawn directly to the display by the layout engine. They are useful for static UI framing (lines, boundaries, unchanging labels).

### Base Primitive Properties
All primitives listed in the `primitives` array accept the following base properties.

| Property | Type | Required | Default | Description |
| -------- | ---- | -------- | ------- | ----------- |
| `type` | Enum (`String`) | Yes | - | The type of shape to draw. |
| `id` | String | No | - | Optional identifier purely for debugging or readability. |
| `isFilled` | Boolean | No | `false` | For closed shapes (`box`, `roundedBox`, `circle`, `triangle`), determines if the shape is filled solid or drawn as an outline. |

### `line`
Draws a 1-pixel wide line between two points.

| Property | Type | Required | Description |
| -------- | ---- | -------- | ----------- |
| `geometry.x1` | Integer | Yes | Starting X coordinate |
| `geometry.y1` | Integer | Yes | Starting Y coordinate |
| `geometry.x2` | Integer | Yes | Ending X coordinate |
| `geometry.y2` | Integer | Yes | Ending Y coordinate |
```json
{
  "type": "line",
  "geometry": { "x1": 0, "y1": 13, "x2": 255, "y2": 13 }
}
```

### `box`
Draws a standard rectangle.

| Property | Type | Required | Description |
| -------- | ---- | -------- | ----------- |
| `geometry.x` | Integer | Yes | Top-left X coordinate |
| `geometry.y` | Integer | Yes | Top-left Y coordinate |
| `geometry.w` | Integer | Yes | Width of the box |
| `geometry.h` | Integer | Yes | Height of the box |
```json
{
  "type": "box",
  "isFilled": false,
  "geometry": { "x": 0, "y": 0, "w": 256, "h": 64 }
}
```

### `roundedBox`
Draws a rectangle with rounded corners.

| Property | Type | Required | Description |
| -------- | ---- | -------- | ----------- |
| `geometry.x` | Integer | Yes | Top-left X coordinate |
| `geometry.y` | Integer | Yes | Top-left Y coordinate |
| `geometry.w` | Integer | Yes | Width of the box |
| `geometry.h` | Integer | Yes | Height of the box |
| `geometry.r` | Integer | Yes | Radius of the rounded corners |
```json
{
  "type": "roundedBox",
  "isFilled": true,
  "geometry": { "x": 10, "y": 10, "w": 50, "h": 30, "r": 3 }
}
```

### `circle`
Draws a geometric circle.

| Property | Type | Required | Description |
| -------- | ---- | -------- | ----------- |
| `geometry.x` | Integer | Yes | Center X coordinate |
| `geometry.y` | Integer | Yes | Center Y coordinate |
| `geometry.r` | Integer | Yes | Radius of the circle |
```json
{
  "type": "circle",
  "isFilled": false,
  "geometry": { "x": 128, "y": 32, "r": 10 }
}
```

### `triangle`
Draws a geometric triangle.

| Property | Type | Required | Description |
| -------- | ---- | -------- | ----------- |
| `geometry.x0` | Integer | Yes | First vertex X coordinate |
| `geometry.y0` | Integer | Yes | First vertex Y coordinate |
| `geometry.x1` | Integer | Yes | Second vertex X coordinate |
| `geometry.y1` | Integer | Yes | Second vertex Y coordinate |
| `geometry.x2` | Integer | Yes | Third vertex X coordinate |
| `geometry.y2` | Integer | Yes | Third vertex Y coordinate |
```json
{
  "type": "triangle",
  "isFilled": true,
  "geometry": { "x0": 0, "y0": 0, "x1": 10, "y1": 5, "x2": 0, "y2": 10 }
}
```

### `text`
Draws a static or system-driven string of text within a bounding box.

| Property | Type | Required | Description |
| -------- | ---- | -------- | ----------- |
| `geometry.x` | Integer | Yes | Top-left X coordinate |
| `geometry.y` | Integer | Yes | Top-left Y coordinate |
| `geometry.w` | Integer | No | Width of the bounding box (-1 for auto) |
| `geometry.h` | Integer | No | Height of the bounding box (-1 for auto) |
| `text` | String | Yes | The text string to render |
| `font` | String | Yes | The font identifier string to use |
| `align` | Integer | No | Alignment (`0`=Left, `1`=Center, `2`=Right) |
| `truncate` | Boolean | No | If true, appends '...' if text exceeds width |

```json
{
  "type": "text",
  "text": "DEPARTURES",
  "font": "NatRailSmall9",
  "align": 0,
  "geometry": { "x": 2, "y": 1 }
}
```

---
*Note: The coordinate system has its origin (0,0) at the top-left of the display.*
