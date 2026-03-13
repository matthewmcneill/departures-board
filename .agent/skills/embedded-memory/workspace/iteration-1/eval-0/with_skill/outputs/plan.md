# Implementation Plan: Refactor BusDataSource to ArduinoJson

## Goal
Replace the manual HTML scraping and `JsonStreamingParser` with `ArduinoJson` for more robust and maintainable parsing of bus departure data.

## Proposed Changes
- Include `ArduinoJson.h`.
- Replace the `while` loop scraper in `updateData` with `deserializeJson(doc, httpsClient)`.
- Map the JSON object fields back to the `stationData` structure.

## Memory Impact Assessment

### Flash / Program Memory (ROM)
- **Estimate Increase**: ~8-12 KB. `ArduinoJson` is a sophisticated library that will increase the code footprint.
- **Impact**: Minimal impact on partition limits, but worth monitoring if other large libraries are added.

### Static RAM
- No significant change. No new global or static variables are required.

### Stack Usage
- **Risk**: High. We must NOT use `StaticJsonDocument` for the arrival data, as the JSON payload from bustimes.org can exceed several kilobytes, risking a stack overflow on the ESP32.
- **Mitigation**: Use `DynamicJsonDocument` or the new `JsonDocument` (ArduinoJson 7+) which allocates on the heap.

### Heap Usage & Fragmentation
- **Peak Usage**: ~4-6 KB during parsing (depending on the number of services returned). This is a temporary peak as the document is destroyed after mapping to `stationData`.
- **Fragmentation Risk**: Moderate. Repeatedly allocating/deallocating `DynamicJsonDocument` could cause fragmentation. 
- **Mitigation**: Reuse a single `JsonDocument` instance as a class member if memory pressure is high, or ensure the allocation is done in a way that allows the heap manager to reclaim the contiguous block.
- **PSRAM**: For very large responses, we could consider allocating the `JsonDocument` buffer in PSRAM.

### Long-term Stability
- **Leaks**: No manual `malloc`/`free` calls are introduced; `ArduinoJson` handles its own memory lifecycle.
- **Low Watermark**: The peak heap usage will temporarily lower the watermark during network updates, but memory should be fully reclaimed immediately after.
