# Remove Padding from Location and Filters Widget

- [x] Create implementation plan
- [x] Remove 8px padding from `locationAndFiltersWidget.cpp`
- [x] Verify fix in simulator
- [ ] Modify `TflService` struct to include `platformName`
- [ ] Update `tflDataSource.cpp` to parse `platformName`
- [ ] Implement multidimensional uniqueness check and "hoisting" in `tflBoard.cpp`
- [ ] Add "Order" index and rename columns in `tflBoard.cpp` and `busBoard.cpp`
- [ ] Fix TfL filter persistence in `webHandlerManager.cpp`
- [ ] Update `layoutDefault.json` for TfL and Bus
- [ ] Regenerate C++ layouts using `gen_layout_cpp.py`
- [ ] Update simulator mock data (`tflBoard.json`, `busBoard.json`)
- [x] Verify padding removal on hardware
- [ ] Verify enriched simulator views in simulator
  - [x] Create implementation plan
  - [ ] Update `tflBoard.json`
  - [ ] Update `busBoard.json`
  - [ ] Verify fix in simulator
