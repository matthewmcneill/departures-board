# JSON Train Formation Ingestion Plan

Based on architectural memory constraints, we will formally bypass building formation parsers inside the memory-sensitive `nrDARWINDataProvider` XML State-Machine entirely. Instead, we will leverage the modernized `nrRDMDataProvider` REST/JSON handler, taking advantage of `ArduinoJson` to perform structural garbage collection.

## Proposed Changes

### 1. Augment `nrRDMDataProvider.cpp` Filter Signature
We will expand the highly specific `ArduinoJson` pre-processor `Filter` dictionary payload to explicitly instruct the memory allocator to keep (not discard) the `formation` objects streamed inside the array of `trainServices`:
```cpp
filter["trainServices"][0]["formation"]["coaches"][0]["coachClass"] = true;
filter["trainServices"][0]["formation"]["coaches"][0]["toilet"]["status"] = true;
```
*(By design in ArduinoJson versions >=6, using `[0]` against an array in a filter applies the template structure allowing extraction against all potential iterations.)*

### 2. Extract Data Only For First Service `(sIdx == 0)`
Within the existing element parser loop iterations, we are already enforcing a strict extraction gateway:
```cpp
if (sIdx == 0) {
   // Currently pulling subsequent Calling Points...
}
```
We will simply add our `formation` retrieval block into this precise `if` statement block. This elegantly ensures that:
1. `firstServiceNumCoaches` is exclusively incremented only for the very first train arriving.
2. The `firstServiceFormation` fixed-array bounds (`NR_MAX_COACHES`) are populated securely without looping off the heap if another train provides 20 random coaches.
3. Once the stream proceeds to `sIdx = 1`, all subsequent formation matrices extracted by `ArduinoJson` in memory are cleanly ignored and dropped out of scope immediately, avoiding struct contamination.

## User Review Required

Does this scoped `ArduinoJson` filter architecture combined with the `if (sIdx == 0)` processing gate precisely match the clever workaround you remember mapping out for constrained ESP bounds? Shall I proceed with editing `nrRDMDataProvider.cpp` directly?
