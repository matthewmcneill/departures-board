# Dynamic Attribution Architecture Refactor Tasks

- [x] Add `getAttributionString()` to `iDataSource.hpp`.
- [x] Implement `getAttributionString()` in `nrDARWINDataProvider`.
- [x] Implement `getAttributionString()` in `nrRDMDataProvider`.
- [x] Implement `getAttributionString()` in `busDataSource`.
- [x] Implement `getAttributionString()` in `tflDataSource`.
- [x] Implement `getAttributionString()` in `weatherClient`.
- [x] Implement `getAttributionString()` in `rssClient`.
- [x] Replace `nrAttribution` usage in `nationalRailBoard.cpp` with dynamic queries.
- [x] Replace `btAttribution` usage in `busBoard.cpp` with dynamic queries.
- [x] Replace `tflAttribution` usage in `tflBoard.cpp` with dynamic queries.
- [x] Automatically inject Weather attribution into the `globalMessagePool` if Weather is enabled.
- [x] Automatically inject RSS attribution into the `globalMessagePool` if RSS is enabled.
- [x] Test code compilation.
