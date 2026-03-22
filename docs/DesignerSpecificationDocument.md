# Software Specification: Departures Board Layout Simulator

## 1. Introduction
### 1.1 Purpose
The Layout Simulator is a standalone, lightweight visualization utility designed to orchestrate the User Interface (UI) layouts for the Departures Board firmware. Its primary objective is to allow developers to build pixel-perfect configurations by dynamically rendering the physical ESP32 OLED display natively within a web browser, providing instantaneous visual feedback as the JSON layout file is edited.

### 1.2 Scope
This tool handles the local rendering of the **View** layer. Instead of a heavy graphical drag-and-drop editor, it acts as a "hardware-in-the-loop" simulator, empowering developers to use existing IDEs (like VS Code) as the layout authoring environment while the visualizer handles instant "hot reload" rendering telemetry via WebSockets.

---

## 2. Architecture & The WASM Engine Pipeline
### 2.1 The "Single Source of Truth" Model
The architecture strictly enforces two distinct sources of truth to separate widget logic from layout configuration:
1. **Widgets (C++ is the Source of Truth):** The visual properties that dictate a widget are authored purely in C++. Developers annotate properties using Doxygen tags (e.g., `@designer_prop bool blinkEnabled`). 
2. **Layouts (JSON is the Source of Truth):** The visual orchestration of a screen is exclusively governed by a `layout_name.json` file. The C++ firmware never hardcodes layouts; it compiles intermediate artifacts derived from this JSON.

### 2.2 The Python Simulator & WebAssembly Pipeline
To prevent "Visual Drift" between the web-based visualization and the physical OLED hardware, the system utilizes a unified WebAssembly layout engine:
* **The Local Server:** A lightweight Python script (e.g., `layout_simulator.py`) runs a local development web server. It uses an OS-level file watcher (like `watchdog`) to monitor the `modules/displayManager/boards/` directory for any modifications to `.json` files.
* **The WASM Compiler:** A predefined build script uses Emscripten to compile the actual C++ `widgets/`, `drawingPrimitives`, and `u8g2` rendering logic into `designer_engine.wasm`.
* **The WebSocket Bridge:** When a developer presses "Save" in their IDE on a `layout_name.json` file, the Python file watcher immediately pushes the new JSON payload over a WebSocket to the browser.
* **The Render Target:** The web UI passes the new JSON coordinates to the embedded WASM payload. The *exact same C++ code* that runs on the ESP32 calculates geometry and pushes pixels to the HTML5 `<canvas>`, guaranteeing 100% hardware parity.

---

## 3. Core Simulator Functional Requirements
### 3.1 The Canvas
* The tool must provide a rigid, non-scrollable workspace locked to `256x64` (or whatever the target hardware resolution is).
* The canvas renders using the `designer_engine.wasm` payload to natively simulate custom fonts (e.g., `Underground` and `NatRail`).

### 3.2 The Hybrid File Navigator
* **Discoverable Dropdown:** When the simulator boots, the Python script traverses the firmware directory and builds a hierarchical web UI dropdown of all available layouts (e.g., `tflBoard -> layoutDefault`). Selecting a file from the dropdown manually triggers a render.
* **Auto-Follow Watcher:** If the developer is making intensive edits in their IDE, the moment they save any JSON layout file within the project, the web UI must dynamically snap its dropdown to that file and instantly hot-reload the canvas. It requires zero manual interaction in the browser to view changes.

### 3.3 The Simulation Dock & Mock Data Integration
To ensure layouts can be accurately evaluated without flashing physical hardware, the Simulator provides dynamic state testing and visual population:
* **Mock Data Injection:** Because the WASM layout simply boots the C++ `GeneratedLayout` class, it boots entirely empty. The Python simulator is responsible for pushing a `mock_data.json` payload into the WASM engine's `updateData()` function on initialization, populating widgets (like the `serviceListWidget`) with realistic fake data to test text truncation and column alignment.
* **The Configuration Toggles:** A small HTML sidebar contains standard firmware configuration toggles (e.g., `[x] Show Clock`, `[ ] Show Date`). Toggling these in the browser sends an instant state-change event to the WASM payload, allowing the developer to visually verify how the layout gracefully handles missing or disabled widgets.
* **Simulated Hardware Brightness:** The HTML sidebar includes a 0-100% slider. Dragging this applies a standard CSS `filter: brightness(X%)` overlay directly to the HTML `<canvas>`. This perfectly emulates the OLED's native hardware PWM dimming for sleep/screensaver states natively in the browser without any C++ compilation overhead.

---

## 4. The "Round-Trip" Export & Compilation Pipeline

The system seamlessly translates the visual JSON layout back into C++ firmware without manual developer intervention.

1. **Authoring Level:** The developer visually designs their board by editing and saving `layout_name.json` within VS Code, visualizing the result instantly in the Simulator.
2. **Intermediate Level:** During the PlatformIO build process, a pre-compile script (`generate_ui.py`) reads the JSON files.
3. **Firmware Level:** The script exports `Generated[Name]Layout.hpp` and `.cpp` files into the transient build directory to be compiled by the ESP32 toolchain. 
    *   **The Hybrid Rendering Approach (Primitives vs Widgets):** To prevent OOP overhead (vtable RAM cost) for simple static structures, the export script treats decorative primitives separately from interactive widgets. 
        *   **Stateful Widgets** (`clockWidget`, `labelWidget`) are instantiated as standard class members.
        *   **Static Primitives** (`line`, `box`) bypass object instantiation entirely. The script injects raw, stateless C++ functions (`drawLine()`, `drawBox()`) directly into the layout's generated `render()` block.
    *   **The Pragmatic Optional Widget Solution:** The script generates a class that instantiates *all* known widgets. However, it statically enforces `isVisible = false` on any widgets the JSON omitted. 
    *   **Memory Impact Justification:** Instantiating the omitted widgets has a functionally zero DRAM footprint vs dynamically allocating them on the heap. This highly pragmatic implementation prevents immense generation complexity.
4. **Reloading Phase:** If a developer alters an existing layout, they **do not** edit `Generated[Name]Layout.cpp`. They reopen the `layout_name.json` in their IDE and Simulator. This paradigm uniquely mirrors the project's native font compilation pipeline, where the `.bdf` configuration is the source of truth, and `fonts.cpp` is a disposable artifact.

---

## 5. Export Schema Definition

The tool operates by maintaining a standard schema `layout_name.json`. The Python script within the PlatformIO build chain detects this JSON and transforms it.

```json
{
  "layout_name": "layoutTflDefault",
  "canvas_w": 256,
  "canvas_h": 64,
  "static_geometry": [
    {"type": "line", "x0": 0, "y0": 15, "x1": 256, "y1": 15},
    {"type": "box", "x": 0, "y": 0, "w": 256, "h": 64, "filled": false}
  ],
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

---

## Appendix A: Future Drag-and-Drop Editor Specifications

*(This section preserves the architectural planning for a full visual Drag-and-Drop Desktop application, should the project scale out of the lightweight Simulator model in the future).*

### A.1 Functional Requirements Expansion
#### The Palette
* Users must be able to drag predefined primitives (e.g., `Line`, `Box`) and complex widgets (e.g., `clockWidget`, `labelWidget`) onto the canvas, populated automatically from the metadata registry cache.

#### The Inspector
* Selecting a widget opens an inspector panel to visually mutate `X`, `Y`, `Width`, `Height`, and `Visibility`.
* Specific widget properties are exposed natively based on the parsed C++ Doxygen tags (e.g. "Blink Enable" for the clock).

### A.2 UI Wireframe Options
#### Layout Option A (Left Toolbar, Right Properties)
This is the standard industry layout for visual editors (Figma, Unity).
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

### A.3 Technology Stack Options
#### Option 1: Web-based (React / Vue + Tauri/Electron + WebAssembly) - **RECOMMENDED**
* **Language:** TypeScript / HTML / CSS / WASM.
* **Pros:** Unbeatable UI development speed. Libraries like `fabric.js` or `Konva.js` make drag-and-drop trivial. Emscripten WASM compilation of the C++ core guarantees pixel-perfect firmware parity. Tauri allows native file system access so the tool can directly read/write `layout_name.json` within the project.
* **Cons:** Massive node_modules folder; slight dependency overhead compared to a pure Python simulator.

*(Other evaluated options like Python/PyQt6 or C++/Dear ImGui were discarded in favor of the WASM+Web stack due to its superior user editing experience combined with automated C++ execution).*
