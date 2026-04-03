# UI Reconciliation Refactor

This refactor successfully transitions the ESP32 `departures-board` firmware from an imperative push-model to a declarative, hash-based reconciliation system. This prevents continuous memory fragmentation and UI flickering by only updating display layout objects when the underlying fetched data structurally changes.

## Changes Made

1. **Hashing Primitive Implementation**
   - Implemented `hashPrimitive` and `hashString` template methods in `iDataSource.hpp` leveraging the extremely lightweight FNV-1a hash algorithm.
   
2. **Bus Board Decoupling**
   - Added `contentHash` field to `BusStop` data structure.
   - Handled FNV-1a hashing against standard telemetry (time, route, destination, ETD, etc) at the conclusion of loop cycles in `BusDataSource::parseResponse()`.
   - Injected UI layout comparison gating using `BusBoard::lastRenderedHash` within the `updateData()` routine.

3. **TfL Board Decoupling**
   - Added `contentHash` field to `TflStation` data structure.
   - Handled hashing across fetched platforms and message queues within `TflDataSource::parseTimeline()`.
   - Enforced reconciliation within `TfLBoard::updateData()` to only reconstruct row objects on state variance.
   
4. **National Rail Board Decoupling**
   - Inserted parallel structure fields, hash accumulation iteration within `executeFetch` sanitization block, and display rendering bypass logic into `NationalRailBoard`.
   - Prevented unnecessary calls to drawing primitives and the truncation of the scrolling marquee effect for identical sequential API pulls.
   
5. **Validation**
   - Performed successful full-stack repository compilation across `esp32dev` and `esp32s3nano` PlatformIO target environments.

## Verification
Please connect your microcontroller unit to test these updates over the serial port!

```bash
pio run -t upload && pio device monitor
```
