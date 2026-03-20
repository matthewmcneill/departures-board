# Software Specification: Departures Board Display Designer

## 1. Introduction
### 1.1 Purpose
The Display Designer is a standalone graphical utility designed to orchestrate the User Interface (UI) layouts for the Departures Board firmware. Its primary objective is to allow developers and designers to drag, drop, and configure visual widgets onto a simulated dot-matrix canvas and seamlessly export the result as highly optimized C++ code for the ESP32.

### 1.2 Scope
This tool handles the **View** layer of the firmware. It outputs the C++ boilerplate required to instantiate widgets and place them mechanically on the screen using the **Lightweight Generated View (LGV)** architectural pattern. It does not generate or interact with the firmware's networking, data-fetching, or business logic. 

---

## 2. System Architecture
### 2.1 The "Single Source of Truth" Model
The designer tool relies on the C++ firmware's widget library as its source of truth. The tool will parse the `iGfxWidget` derivatives available in the project to populate its UI palette.

### 2.2 Export Pipeline
1. **Designer Level:** The user saves the project as `layout_name.json`.
2. **Intermediate Level:** A pre-compile script (`generate_ui.py`) reads the JSON.
3. **Firmware Level:** The script exports `Generated[Name]View.hpp` and `.cpp` files. To solve the Optional Widget Problem without breaking compilation, the script generates *all* known widgets but enforces `isVisible = false` on those the designer did not actively place on the canvas. 

---

## 3. Core Functional Requirements
### 3.1 The Canvas
* The tool must provide a rigid, non-scrollable workspace locked to `256x64` (or whatever the target hardware resolution is).
* The canvas must snap widgets to a pixel grid to emulate the dot-matrix display.
* The canvas should simulate the `Underground` and `NatRail` fonts natively in the tool.

### 3.2 The Palette
* Users must be able to drag predefined primitives (e.g., `DrawBox`, `DrawLine`) and complex widgets (e.g., `clockWidget`, `serviceListWidget`) onto the canvas.

### 3.3 The Inspector
* Selecting a widget opens an inspector panel to mutate `X`, `Y`, `Width`, `Height`, and `Visibility`.
* Specific widgets should have exposed properties (e.g. "Blink Enable" for the clock).

---

## 4. UI Wireframe Options

### Layout Option A (Left Toolbar, Right Properties)
This is the standard industry layout (Figma, Unity).
```text
+-----------------------------------------------------------------+
| File  Edit  Export Layout                                       |
+-----------+---------------------------------------+-------------+
| PALETTE   |                CANVAS                 | PROPERTIES  |
|           |                                       |             |
| [Text]    |   +-------------------------------+   | ID: clock_1 |
| [Clock]   |   | Layout: tfl_default           |   | ----------- |
| [List]    |   |                               |   | X: 200      |
| [Marquee] |   |   1 Brixton      2 mins       |   | Y: 0        |
|           |   |                           [ ] |---| W: 56       |
| [Box]     |   |                               |   | H: 14       |
| [Line]    |   |        20:50:06               |   | Font: Und10 |
|           |   +-------------------------------+   | Visible: [x]|
|           |                                       | Blink: [x]  |
|           |                                       |             |
|-----------+---------------------------------------+-------------|
| HIERARCHY | > Exporting to /boards/tflBoard/layouts/            |
| - Clock   | > JSON Validated.                                   |
| - List1   |                                                     |
+-----------+-----------------------------------------------------+
```

### Layout Option B (Bottom Timeline / Tabbed Layout)
Useful if animations (like scrolling speed timelines) are added later.
```text
+-----------------------------------------------------------------+
| [Save] [Undo] [Redo] [Export to C++]                            |
+---------------------------------------+-------------------------+
|                CANVAS                 |        INSPECTOR        |
|                                       |       [Widget] [Canvas] |
|   +-------------------------------+   |                         |
|   | 1 Brixton      2 mins         |   | Widget: serviceList     |
|   | 2 Brixton      7 mins         |---| X: 0     Y: 15          |
|   |                               |   | W: 256   H: 26          |
|   |        20:50:06               |   | Max Rows: 2             |
|   +-------------------------------+   | Columns: 3              |
|                                       +-------------------------+
|                                       | HIERARCHY               |
|                                       |  [-] Screen             |
|                                       |   |- serviceListWidget  |
|                                       |   |- clockWidget        |
+---------------------------------------+-------------------------+
```

---

## 5. Technology Stack Options

To build the Designer Tool, three primary framework options are structurally viable.

### Option 1: Web-based (React / Vue + Tauri/Electron) - **RECOMMENDED**
* **Language:** TypeScript / HTML / CSS.
* **Pros:** Unbeatable UI development speed. Libraries like `fabric.js` or `Konva.js` make canvas drag-and-drop trivial. Electron or Tauri allows native file system access so the tool can easily read `iGfxWidget.hpp` and write C++ directly back to the project folder.
* **Cons:** Massive node_modules folder; slight dependency overhead.

### Option 2: Python Desktop (PyQt6 / Tkinter)
* **Language:** Python
* **Pros:** Perfectly integrates with your existing PlatformIO Python pre-build scripts. No node.js required. Natively handles file system parsing easily.
* **Cons:** Building modern, slick drag-and-drop canvas UIs in Tkinter or PyQt can feel clunky and visually dated compared to Web technologies.

### Option 3: Immediate Mode GUI (C++ / Dear ImGui)
* **Language:** C++
* **Pros:** You write the designer tool in the exact same language as the firmware. You can actually compile your ESP32 widgets natively for Mac/Windows and have the designer tool render *the exact same C++ rendering code* on your desktop screen as a pixel-perfect emulator.
* **Cons:** Requires setting up SDL2 / OpenGL build toolchains. Very steep learning curve for building complex desktop apps compared to Web.

## 6. Export Schema Definition

The tool operates by writing a standard schema `layout_name.json` to the target board's directory. 
The Python script within the PlatformIO build chain detects this JSON and translates it.

```json
{
  "layout_name": "tfl_default",
  "canvas_w": 256,
  "canvas_h": 64,
  "widgets": [
    {
      "id": "main_clock",
      "type": "clockWidget",
      "x": 200, "y": 0, "w": 56, "h": 14,
      "properties": {
        "font": "UndergroundClock8",
        "blinkEnabled": true,
        "isVisible": true
      }
    },
    {
      "id": "train_list",
      "type": "serviceListWidget",
      "x": 0, "y": 15, "w": 256, "h": 26,
      "properties": {
        "isVisible": true
      }
    }
  ]
}
```
