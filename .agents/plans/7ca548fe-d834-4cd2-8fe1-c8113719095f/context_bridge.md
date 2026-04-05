# Context Bridge: Train Formations Widget

## 📍 Current State & Focus
We have completed the architectural planning and memory optimizations for the Train Formations Widget. The `implementation_plan.md` has been updated with ASCII designs and memory-saving constraints. We successfully moved the formation array to the `NationalRailStation` struct (as `firstServiceFormation`) to save significant heap overhead. We used `tools/bdfconv` to compile the user-provided `FormationIcons7.bdf` into a C-array, which is temporarily staged in `.agents/tmp/FormationIcons7.c`.

## 🎯 Next Immediate Actions
1. **Font Integration**: Append the array in `.agents/tmp/FormationIcons7.c` into `modules/displayManager/fonts/fonts.cpp` (the `extern` was already added to `fonts.hpp`).
2. **Widget Development**: Create `modules/displayManager/widgets/trainFormationWidget.hpp` and `trainFormationWidget.cpp`. Implement the static and marquee animation state-machine modes as detailed in the implementation plan.
3. **Data Binding**: Wire up the mock arrays in `tools/layoutsim/mock_data/nationalRailBoard.json` to safely test the rendering engine in WASM.

## 🧠 Decisions & Designs
- **Memory Footprint**: `NrCoachFormation[NR_MAX_COACHES]` is stored *only* for the first service in the station struct, saving roughly 5KB of static allocation.
- **Sizing Constraints**: Minimum coach carriage width is strictly set to **15px** to accommodate the broad 12px bicycle glyph plus 1px frame padding on each side.
- **Animation Strategy**: A 3-state rotating loop (Number -> Facilities -> Loading) cycling every 3 seconds. A Marquee sweep activates instead if the total train length exceeds the widget layout bounds.

## 🐛 Active Quirks, Bugs & Discoveries
- The user is currently running background compilations (`pio run`) frequently, and there may be other concurrent sessions managing other tasks (e.g. Settings Backup). ALWAYS verify lock viability before triggering builds.

## 💻 Commands Reference
- Run layout simulation: `python3 tools/layoutsim/scripts/dev_server.py`
- Generate layouts: `python3 tools/layoutsim/scripts/gen_layout_cpp.py`

## 🌿 Execution Environment
- Target: Local ESP32 Layout Simulator / Web UI
- Portability Rule: All file references must remain strictly repository-relative.
