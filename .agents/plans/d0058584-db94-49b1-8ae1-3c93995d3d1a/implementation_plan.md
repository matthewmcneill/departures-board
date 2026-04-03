- [x] Reviewed by house-style-documentation - passed (camelCase, std sections)
- [x] Reviewed by architectural-refactoring - passed (Decouples Data from View effectively)
- [x] Reviewed by embedded-systems - passed (FNV-1a ensures zero new allocations, prevents heap fragmentation, uses only 4 bytes of RAM per board).

# Hash-Based UI Data Reconciliation Pattern

Migrate the state machine updates from an imperative event-based pattern (`UpdateStatus::SUCCESS` triggering layout rebuilds) to a declarative synchronization pattern. The Data Sources will compute an `fnv1a` hash over ALL meaningful visual data elements, including the dynamically managed Message Pools. The Display Boards will store a `lastRenderedHash` and only rebuild dynamic UI strings and layouts when the content hashes differ. This prevents unnecessary display flickering and completely zeroes out heap fragmentation during redundant fetch cycles.

## User Review Required

Please review the comprehensive scope of data fields evaluated for hashing under each section below to ensure no displayable data points have been missed. 

## Proposed Changes

---

### Shared Data Source Implementation

#### [MODIFY] modules/dataManager/iDataSource.hpp
* Add protected FNV-1a hashing primitives directly to the `iDataSource` abstract base class to avoid new utility files while keeping them accessible to the specialized subclasses:
```cpp
protected:
    uint32_t hashString(const char* str, uint32_t hash = 2166136261u) {
        if (!str) return hash;
        while (*str) {
            hash ^= (uint8_t)(*str++);
            hash *= 16777619u;
        }
        return hash;
    }

    template <typename T>
    uint32_t hashPrimitive(const T& val, uint32_t hash = 2166136261u) {
        const uint8_t* p = reinterpret_cast<const uint8_t*>(&val);
        for (size_t i = 0; i < sizeof(T); i++) {
            hash ^= p[i];
            hash *= 16777619u;
        }
        return hash;
    }
```

---

### Bus Board Modifications

#### [MODIFY] modules/displayManager/boards/busBoard/busDataSource.hpp
* Add `uint32_t contentHash = 0;` to the `BusStop` data struct.

#### [MODIFY] modules/displayManager/boards/busBoard/busDataSource.cpp
* At the successful conclusion of parsing (likely inside `executeFetch()`), compute the hash iteratively over all UI data:
  * Station Data: Hash `stationData->location` and `stationData->numServices`.
  * Iterator over `stationData->service` and run `hashString` on: `sTime`, `expectedTime`, `destination`, `routeNumber`.
  * Iterate over `messagesData` (`MessagePool`) returning `messagesData.getMessage(i)` and applying `hashString`.
* Store the resulting cascaded hash value in `stationData->contentHash`.

#### [MODIFY] modules/displayManager/boards/busBoard/busBoard.hpp
* Add `uint32_t lastRenderedHash = 0;` to the `BusBoard` class.

#### [MODIFY] modules/displayManager/boards/busBoard/busBoard.cpp
* Within `BusBoard::updateData()`, wrap the `activeLayout->servicesWidget.clearRows()` and `addRow()` layout reconstruction tightly around an equality check: `if (data->contentHash != lastRenderedHash)`.
* Update `lastRenderedHash` upon completion of the layout generation.

---

### National Rail Board Modifications

#### [MODIFY] modules/displayManager/boards/nationalRailBoard/nationalRailDataSource.hpp
* Add `uint32_t contentHash = 0;` to the `NationalRailStation` struct.

#### [MODIFY] modules/displayManager/boards/nationalRailBoard/nationalRailDataSource.cpp
* Compute the `fnv1a` hash over ALL fields passed to the view:
  * Station Data: `location`, `stationName`, `filterVia`, `filterPlatform`, `platformAvailable`, `numServices`.
  * Iterate `NationalRailService` records and hash: `sTime`, `destination`, `via`, `etd`, `platform`, `isCancelled`, `isDelayed`, `trainLength`, `classesAvailable`, `opco`, `calling`, `origin`, `lastSeen`, `serviceMessage`, `serviceType`.
  * Iterate `messagesData` and run `hashString()` on all disruption strings.
* Run the computation at the end of the background SOAP parse. 

#### [MODIFY] modules/displayManager/boards/nationalRailBoard/nationalRailBoard.hpp
* Add `uint32_t lastRenderedHash = 0;` to `NationalRailBoard`.

#### [MODIFY] modules/displayManager/boards/nationalRailBoard/nationalRailBoard.cpp
* Apply reconciliation logic inside `updateData()`. Clear and reconstruct UI rows on hash mismatch.

---

### TfL Board Modifications

#### [MODIFY] modules/displayManager/boards/tflBoard/tflDataSource.hpp
* Add `uint32_t contentHash = 0;` to the `TflStation` struct.

#### [MODIFY] modules/displayManager/boards/tflBoard/tflDataSource.cpp
* Compute the `fnv1a` hash over ALL fields passed to the view:
  * Station Data: `location`, `numServices`.
  * Iterate `TflService` and hash: `destination`, `lineName`, `expectedTime`, `timeToStation`.
  * Iterate over `messagesData` from the message pool and run `hashString()`.
* Run the computation at the end of the JSON parse cycle. 

#### [MODIFY] modules/displayManager/boards/tflBoard/tflBoard.hpp
* Add `uint32_t lastRenderedHash = 0;` to `TflBoard`.

#### [MODIFY] modules/displayManager/boards/tflBoard/tflBoard.cpp
* Same reconciliation logic inside `updateData()`. Clear and reconstruct UI rows on hash mismatch.

## Verification Plan

### Manual Verification
- Flash and observe `.pio` monitor. `[VERB] DataManager: Data update finished successfully` logs should continue to stream every 30-60s.
- The UI should visibly NEVER flicker or stutter on consecutive identical data fetch successes because the widget clear/allocation cycle is bypassed.
- Upon a route timing changing (e.g. `2 mins` drops to `1 min`) OR a new message populating the feed, the UI should reconstruct seamlessly and animate out.
