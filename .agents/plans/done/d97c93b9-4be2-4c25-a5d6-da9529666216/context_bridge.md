# Context Bridge

## 📍 Current State & Focus
- The project is investigating an Out-Of-Memory (OOM) crash that happens when adding multiple National Rail boards from the Web UI. Adding a board permanently locks ~20 KB of heap due to the massive `NationalRailStation` struct size holding caches for 9 trains. 
- A new implementation plan was successfully approved to extract the largest arrays (`calling`, `origin`, `lastSeen`, `serviceMessage`) out of the `NationalRailService` struct and put them in `NationalRailStation` solely for the **"next train"** (Service 0). This drops the payload footprint drastically and saves ~15 KB per hardware board.
- The user also requested and approved wrapping critical memory allocations in `BoardFactory` with `try/catch` logic to ensure the system gracefully skips rendering rather than crashing `AsyncTCP` connections.

## 🎯 Next Immediate Actions
1. Re-read the `implementation_plan.md` to refresh on the exact structural changes to `modules/displayManager/boards/nationalRailBoard/nationalRailDataSource.hpp`.
2. Edit `modules/displayManager/boards/nationalRailBoard/nationalRailDataSource.cpp` and `nationalRailBoard.cpp` to pipe the data to the correct properties as outlined in the plan.
3. Edit `modules/displayManager/boardFactory.cpp` and `modules/webServer/webHandlerManager.cpp` to introduce `try/catch(const std::bad_alloc& e)` structures where `new` or `std::make_unique` are heavily used.
4. Flash the device firmware and test allocating multiple boards simultaneously.

## 🧠 Decisions & Designs
- We explicitly decided **NOT** to drop the `NR_MAX_CALLING` character limit from 450 to 255. Instead, we preserve the capacity but reduce allocations by only keeping a single 450-byte buffer at the Station level, rather than inside the structural array for trains 1-8.
- Added strict defensive programming parameters for dynamic heap object instantiation.

## 🐛 Active Quirks, Bugs & Discoveries
- ESPAsyncWebServer threads receive background payload requests independent of the main `loop()`. If the heap is pushed below a few hundred bytes, `new` inside `.pio/libdeps/esp32dev/AsyncTCP` crashes the whole chip with `std::terminate`. Protecting our explicit board allocations avoids entering this threshold altogether.

## 💻 Commands Reference
- `/flash-test` cleanly handles destroying active serial connections and flashing the ESP32.

## 🌿 Execution Environment
Hardware attached, ESP32 target platform.
