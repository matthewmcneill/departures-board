# Context Bridge

- **📍 Current State & Focus**: Just finished researching and drafting an architectural plan to migrate the user interface updates from an imperative push (`UpdateStatus::SUCCESS`) to a declarative synchronization pattern. The new pattern depends on hashing the parsed data (Bus, TfL, NR) using a templated FNV-1a primitive `hashPrimitive` natively injected into the `iDataSource` abstract base class. The implementation plan has been reviewed and approved by the user.
- **🎯 Next Immediate Actions**: Use `/plan-start` to claim the hardware lock. Follow the implementation plan to edit `iDataSource.hpp` and insert the templated `hashPrimitive`. Add `contentHash` tracking to the `BusStop`, `NationalRailStation`, and `TflStation` structs. Invoke the cascaded hash in the `executeFetch` completion loops of the 3 Data Sources, and rewrite the `updateData()` method inside the display boards to rebuild the UI layout rows exclusively upon a `contentHash` delta.
- **🧠 Decisions & Designs**: 
  - FNV-1a non-cryptographic hashing will be used.
  - The hash function will be a unified templated generic `hashPrimitive` attached as a protected member directly on the `iDataSource` parent class rather than scattered utility files. This naturally handles `char*`, integers, `bool`, bytes, `enum`, and future floats elegantly.
- **🐛 Active Quirks, Bugs & Discoveries**: `MessagePool` feeds need manual iteration over `getMessage(i)` inside the data sources to fold the disruption messages firmly into the underlying data hash cascade.
- **💻 Commands Reference**: 
  - Standard Build: `pio run`
  - Flash Hardware: `pio device upload`
  - Monitor Device Logs: `pio device monitor`
- **🌿 Execution Environment**: Standard `departures-board` target connected to `pio`. 
