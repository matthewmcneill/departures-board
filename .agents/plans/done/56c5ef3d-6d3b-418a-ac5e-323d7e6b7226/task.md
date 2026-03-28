# Network Data Architecture Implementation Plan

- [x] Draft initial `implementation_plan.md` detailing the class modifications for `dataManager` and `iDataSource`
- [x] Run the `/review-ip` workflow to ensure skill compliance (Memory, Power, House Style)
- [x] Present `implementation_plan.md` to user for approval

## Execution Phase
- [x] Rename `modules/dataWorker/` to `modules/dataManager/` and inside files.
- [x] Modify `iDataSource.hpp` with new interface methods.
- [x] Implement `dataManager.hpp` and `dataManager.cpp` priority scheduling loop.
- [x] Update `NationalRailDataSource` and `TflDataSource` to implement logic.
- [x] Update `BusDataSource` to implement logic.
- [x] Update `weatherClient` and `appContext` integration.
- [x] Integrate `rssClient` natively into DataManager schedulng
- [x] Compile and verify via `pio run`.

## Debugging Phase (Crashes)
- [x] Debug Core 1 `sysManager` UI hang. Root cause: Display threads interacting with DataManager output without Mutex locks leading to LoadProhibited exceptions. Added `lockData()` to `iDataSource`.
- [x] Debug DataManager portal tests crash. Root cause: Priority queue dropped transient source requests causing AsyncTCP to block infinitely.
- [x] Refactored DataManager loop to utilize a single execution block natively tracking priority queue interrupts.
