# Layout Simulator Debugging Task

- [x] Investigate empty drop-downs for display selection
  - [x] Locate frontend UI code for the simulator
  - [x] Determine how display selection options are loaded (e.g. from JSON or API)
  - [x] Identify failure point (Currently looking at dev_server.py finding 0 files)
- [x] Investigate incorrect widget list for the displays
  - [x] Review `getLayoutMetadata` in WASM or JS parsing logic
  - [x] Check if `layoutParser` validation status is returning effectively (Found issue in gen_sim_registry.py mapping board layouts to layout class names instead of layout JSON id)
- [x] Investigate why no data is being displayed
  - [x] Check if `renderFrame` or `syncData` is failing
  - [x] Check memory buffer extraction in `main.cpp`
  - [x] Check U8g2 rendering (The root cause is the same as above, no widgets are instantiated in active profile)
- [x] Draft an implementation plan with proposed fixes
- [x] Review proposed fixes against project rules
- [x] Apply fixes and confirm simulator behavior

- [x] Investigate why data/graphics are not rendering on the canvas
  - [x] Check WASM buffer rendering loop `renderFrame`
  - [x] Check U8g2 SH1122 buffer unpacking logic in `main.cpp`
  - [x] Check `layoutParser->render(*g_u8g2)` logic

- [x] Revert `clockWidget` text rendering to native `drawStr`
- [x] Audit entire widget library and inject `U8g2StateSaver` RAII guards where necessary

## 3. Verification
- [x] Run simulator validation to capture rendering tests.
- [x] Verify no text overlaps active static layout widgets.
- [x] Document reference mid-scroll frames into final walkthrough.

- [x] Run the WASM layout simulator and capture visual proof of the refactored displays
