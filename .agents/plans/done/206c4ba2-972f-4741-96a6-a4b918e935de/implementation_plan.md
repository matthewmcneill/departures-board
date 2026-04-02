# Centralized Data Worker Queue Implementation

## Goal Description
Refactor the background data fetching architecture to use a "Centralized Data Worker Queue" (Option B). Currently, each data source (`weatherClient`, `nationalRailDataSource`, etc.) spawns its own FreeRTOS task on Core 0 when data is requested. If multiple requests occur simultaneously, this leads to concurrent TLS handshakes, which dramatically spikes heap usage (potentially causing Out-Of-Memory errors) and causes CPU starvation on Core 0.

The new architecture will introduce a single, permanent FreeRTOS worker task on Core 0. Data sources will submit fetch requests to a FreeRTOS queue (`xQueue`), and the worker will process them sequentially. This guarantees that only one TLS buffer is allocated at a time, ensuring peak memory stability, while keeping network operations completely asynchronous from the UI core.

## User Review Required

> [!NOTE]
> *User Feedback Resolved:* The standard FIFO (First-In-First-Out) queue approach has been deemed acceptable for the current UX. Priority queuing is not required at this time.
> 
> *Architecture Decision - Module vs Lib:* `dataWorker` will be implemented as a **Module** (`modules/dataWorker/`). The `lib/` directory in this project is reserved for highly generic, decoupled utilities (like `xmlStreamingParser` or `md5Utils`). Because `dataWorker` must interact specifically with the application's `iDataSource` interface to execute the fetches, it represents application-specific orchestration logic, fitting the definition of a module.

## Resource Impact Assessment

As required by the `@embedded-systems` skill, here is the impact assessment for this architectural change:

### Memory (Flash/RAM/Stack/Heap)
- **Heap Usage (Critical Improvement)**: Peak heap usage will drop significantly. Instead of potentially allocating 3x 40KB TLS buffers concurrently (120KB peak), the system will structurally guarantee a maximum of 1x 40KB concurrent TLS allocation.
- **RAM (Static)**: We will add a small static `xQueueHandle` and a small static array for the queue items (e.g., 10 items * 8 bytes = 80 bytes).
- **Stack**: We replace multiple 8KB localized task stacks with a single 8KB stack for the dedicated `DataWorker` task. This saves ~16KB of FreeRTOS Task overhead in RAM.
- **Flash**: Minimal impact.

### Power
- **Impact**: Slight improvement. Sequentially utilizing the Wi-Fi radio and CPU Core 0 for encryption is generally more power-efficient than thrashing the RTOS scheduler with competing high-load tasks.

### Security
- **Impact**: Neutral. TLS 1.2+ remains untouched. By preventing concurrent Out-Of-Memory (OOM) crashes, the system is less susceptible to Denial of Service via rapid, concurrent triggering of API calls.

## Proposed Changes

---

### System Documentation

#### [MODIFY] [SystemSpecificationDocument.md](docs/SystemSpecificationDocument.md)
Update Section 4 (Data Design) and Section 5.10 (`iDataSource`) to document the new Centralized Worker Queue.
- Add clear design rationale on *why* this approach was chosen (Heap peak minimization, preventing LwIP socket exhaustion).
- Detail the flow: `updateData()` enqueues a request pointer -> `DataWorker` dequeues and calls `executeFetch()` -> Task complete.

---

### Core Application Logic

#### [NEW] `modules/dataWorker/dataWorker.hpp`
#### [NEW] `modules/dataWorker/dataWorker.cpp`
Create a new `dataWorker` module responsible for owning the FreeRTOS Queue and the pinned Core 0 task.
- **Queue Structure**: A simple struct containing a pointer to the calling `iDataSource` instance.
- **Task Loop**: A permanent task (`tskIDLE_PRIORITY + 1`) on Core 0 that blocks on `xQueueReceive`. When an item is received, it calls the virtual `executeFetch()` method on the provided data source pointer.

#### [MODIFY] [appContext.hpp](modules/appContext/appContext.hpp)
#### [MODIFY] [appContext.cpp](modules/appContext/appContext.cpp)
- Instantiate and initialize the `dataWorker` during the boot sequence.

---

### Data Sources Implementation

#### [MODIFY] [iDataSource.hpp](modules/displayManager/boards/interfaces/iDataSource.hpp)
- Add a virtual `executeFetch()` method that the `dataWorker` can call. Currently, elements like `weatherClient` and `nationalRailDataSource` implement this internally but it must be exposed (or exposed via a friend/interface wrapper) so the worker can trigger it.

#### [MODIFY] [weatherClient.hpp](modules/weatherClient/weatherClient.hpp)
#### [MODIFY] [weatherClient.cpp](modules/weatherClient/weatherClient.cpp)
- Remove `xTaskCreatePinnedToCore` from `updateData()`.
- Instead, submit `this` to the `dataWorker` queue.
- Add comprehensive Doxygen-style header comments detailing the architectural requirement of the worker queue to prevent heap exhaustion.

#### [MODIFY] [nationalRailDataSource.hpp](modules/displayManager/boards/nationalRailBoard/nationalRailDataSource.hpp)
#### [MODIFY] [nationalRailDataSource.cpp](modules/displayManager/boards/nationalRailBoard/nationalRailDataSource.cpp)
- Remove `xTaskCreatePinnedToCore` from `updateData()`.
- Instead, submit `this` to the `dataWorker` queue.
- Add comprehensive Doxygen-style header comments detailing the architectural requirement of the worker queue.

#### [MODIFY] `tflDataSource` and `busDataSource`
- Apply the identical queue-submission refactoring to all other data sources to ensure uniformity across the HSM data fetch triggers.

## Verification Plan

### Automated Tests
- Run `pio run -e unit_testing_host` to ensure no syntax/inheritance breakage. (Though hardware FreeRTOS queues won't easily run on native, we will ensure it compiles).

### Manual Verification
1. Flash to ESP32s3 target.
2. Configure a display carousel that includes both National Rail and Weather.
3. Observe the Serial Monitor (`pio device monitor`) during boot/carousel transition.
4. Validate that `DataWorker` receives requests and processes them sequentially, eliminating the overlapping `[DATA]` logs for TLS handshakes.
5. Watch the `Free Heap` telemetry to ensure it never dives below 50KB during parallel logical board updates.
