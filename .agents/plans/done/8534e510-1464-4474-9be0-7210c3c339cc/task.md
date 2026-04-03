# Decouple National Rail Initialization from UI

- [x] Remove `WiFi.h` and initialization checks from `nationalRailBoard.cpp`
- [x] Add `isInitialized()` method to `nationalRailDataSource.hpp`
- [x] Refactor `nationalRailDataSource::executeFetch()` to safely self-initialize
- [x] Verify clean compilation
- [ ] Complete session logging and cleanup
