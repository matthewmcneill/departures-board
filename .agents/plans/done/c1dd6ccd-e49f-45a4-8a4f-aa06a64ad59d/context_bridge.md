# Context Bridge: c1dd6ccd-e49f-45a4-8a4f-aa06a64ad59d

## 📍 Current State & Focus
We successfully resolved a major WebAssembly layout engine discrepancy! The simulator was incorrectly rendering physical metrics widgets (like `weatherWidget` and `wifiWarning`) inside custom subsets (e.g., `layoutGadec.json`) when they were not implicitly scoped by the IDE JSON config file.
The simulator JS code was overriding undefined properties internally with `visible: true`, which masked the missing elements from the DOM rendering logic but still displayed them on the underlying C++ OLED cache buffer. This mismatch between IDE properties and WebAssembly parsing has been fundamentally fixed mimicking the "Pass 1.5" auto-hide technique built into native ESP32 compiles. 

Additionally, we modified the `dataHarvester.py` scripts to manufacture a mocked long string array for `.json` due to 500 Internal Server errors from National Rail's Huxley proxy API endpoint limiting live tests. 

## 🎯 Next Immediate Actions
- The immediate bugs from the layout discrepancy between Native layout `.cpp` compiling vs. Simulator IDE have been fixed. You can continue iterating the project or UI layouts.

## 🧠 Decisions & Designs
- **Default Visibility Model:** WebAssembly Simulator widgets (`layoutParser.hpp`) are forced to `visible = false` early on initialization. Only elements that are dynamically processed natively via the `arduinoJSON` layer will restore themselves to visibility via their `widget->setVisible(true)`. 
- **Web IDE Diagnostics:** `main.cpp` was modified explicitly to export `w["visible"] = widget->getVisible();` rather than leaving it loosely implied, bridging the gap between C++ logic state and IDE frontend JS state. `index.html` leverages this boolean parameter specifically when drawing HTML boxes.

## 🐛 Active Quirks, Bugs & Discoveries
- **National Rail Huxley API Extinction Events**: The Huxley2 API proxy `expand=true` endpoints are prone to erratic 500 Outages for large data structures during peak hours. Be extremely mindful when running `python3 scripts/dataHarvester.py` since it could crash unexpectedly depending on real-world rail conditions. We currently bypassed it by pushing to `tools/layoutsim/mock_data/nationalRailBoard.json` manually.

## 💻 Commands Reference
- WASM Simulator Builder: `python3 tools/layoutsim/scripts/build_wasm.py`
- Harvester: `python3 scripts/dataHarvester.py --station MAN --out nationalRailBoard.json`
- Native ESP32 Tester: `pio run -e esp32dev`

## 🌿 Execution Environment
- Testing mainly conducted in the Layout Simulator via `npx serve` and WebAssembly `build_wasm.py`. Physical tests succeeded previously on ESP32 target.
