# Context Bridge

**📍 Current State & Focus**
The "loading data" stall issue across all display boards has been successfully resolved. The root cause was a FreeRTOS `priorityEventQueue` dangling pointer sequence exposed when configuration data triggered board destructions safely, but orphaned data source pointers continued circulating inside the DataManager's worker queue.
We have modified `modules/dataManager/dataManager.cpp` to validate that any `eventSource` drawn from the message queue is actively registered within the `manager->registry`. Orphaned events are now safely discarded, resolving the infinite backoff deadlocks.

**🎯 Next Immediate Actions**
None from the context of this issue—this branch's technical debt has been structurally stabilized and verified against actual ESP32 hardware via serial logs.

**🧠 Decisions & Designs**
- **Validation Before Execution Check:** Added `std::find(manager->registry.begin(), manager->registry.end(), eventSource)` to `DataManager::workerTaskLoop`.
- **Dangling Pointer Graceful degradation:** Orphaned queue elements now emit a `WARNING` log (Discarded orphaned queue event) but will silently fallback to executing the current `bestSource` polling metric to avoid blocking the priority queue.

**🐛 Active Quirks, Bugs & Discoveries**
- DataManager's scheduling loop on Core 0 pushes a 15-second sleep (`now + 15000`) if a data polling function doesn't update its own pointer's `nextFetchTime`.
- NTP network handshakes can take up to ~50 seconds in the initialization phase initially on boot if the connection is slow. `appContext::tick()` will visually stall the LoadingBoard until this succeeds.

**💻 Commands Reference**
- Build/Upload: `pio run -t upload`
- Serial Monitoring test: `pio device monitor -e esp32dev`

**🌿 Execution Environment**
- Branch: `refactor/technical-debt`
- Target: Hardware ESP-32

**⚠️ PORTABILITY RULE**
File structures and references adhere to relative repositories, such as `modules/dataManager/dataManager.cpp`.
