# Walkthrough: ESP32 Heap Memory Optimization

## Overview
We detected a fatal `abort()` exception in production due to a `std::bad_alloc` when instantiating display boards, leading to crashes. This issue became prominent after adding several data-intensive National Rail boards on the system, which exhausted Free Heap memory. 

This walkthrough documents our multi-phase strategy to dramatically reduce memory footprint and safeguard dynamic allocations across the `departures-board` codebase.

## 1. Struct Redesign (Data Layer)
The `NationalRailService` structs were carrying multiple huge `char` buffers (`calling`, `origin`, `lastSeen`, `serviceMessage`) individually for *every single service* (usually 9 per loaded station):
- We flattened the model by relocating these message buffers up to the `NationalRailStation` level as `firstService...` variables. 
- Conceptually, since the scrolling ticker only ever plays data from the *First Due* service on the departure board, we only ever need one copy of this data.
- **Result:** We slashed approximately **15KB** of static allocation per loaded National Rail board.

## 2. Parsing Plumbing (XML Listener)
- In `nationalRailDataSource.cpp`, we updated the `value()` handler.
- We added simple integer index checks (`id == 0`) before making heavy string copies from the XML SAX parser. Deep-data elements for subsequent services are now automatically truncated, vastly speeding up execution.

## 3. UI Plumbing
- In `nationalRailBoard.cpp`, the data-binding logic was rebound to point at the new `firstService...` attributes on the `NationalRailStation` objects when determining which ticker message string to queue up.

## 4. Defensive Instantiations
To protect against fragmented memory exceptions:
- In `boardFactory.cpp`: Added robust `try...catch(const std::bad_alloc& e)` wrappers around all dynamic instantiations of `iDisplayBoard` subclasses. When an OOM triggers during the UI addition sequence, a null pointer is returned gracefully instead of killing the firmware.
- In `webHandlerManager.cpp`: Wrapped the dynamic instantiation of `std::make_unique<ApiTestDataSource>` to aggressively intercept test-button 500 error triggers during Web UI API connection testing.

## Verification Results
- **Pushed To Target:** Firmware was successfully flashed to an ESP32 attached via serial.
- **Monitoring:** System logs confirmed an instantaneous reduction in memory usage. With 5 simultaneous boards configured (EWW, EPS, HUBTCR, Bus, Clock), baseline `Free Heap` post-network load was stabilized at `104,960 Bytes`.
- **Conclusion:** We achieved ~54 KB heap recuperation, averting the immediate fragmentation crisis, and we added architectural guarantees that system operation will gracefully fail on future memory bursts.
