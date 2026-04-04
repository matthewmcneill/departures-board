# Dynamic Attribution Architecture Refactor Tasks

- [ ] Add `getAttributionString()` to `iDataSource.hpp`.
- [ ] Implement `getAttributionString()` in `nrDARWINDataProvider`.
- [ ] Implement `getAttributionString()` in `nrRDMDataProvider`.
- [ ] Implement `getAttributionString()` in `busDataSource`.
- [ ] Implement `getAttributionString()` in `tflDataSource`.
- [ ] Implement `getAttributionString()` in `weatherClient`.
- [ ] Implement `getAttributionString()` in `rssClient`.
- [ ] Replace `nrAttribution` usage in `nationalRailBoard.cpp` with dynamic queries.
- [ ] Replace `btAttribution` usage in `busBoard.cpp` with dynamic queries.
- [ ] Replace `tflAttribution` usage in `tflBoard.cpp` with dynamic queries.
- [ ] Automatically inject Weather attribution into the `globalMessagePool` if Weather is enabled.
- [ ] Automatically inject RSS attribution into the `globalMessagePool` if RSS is enabled.
- [ ] Test code compilation.
