---
name: "Aligning Bus and Tube Widgets to National Rail Pattern"
description: "Unified the widget architecture across National Rail, Tube, and Bus boards by decoupling the service list. Split the list into a static header `row0Widget` and a dynamic scrolling `servicesWidget`. In..."
created: "2026-04-03"
status: "DONE"
commits: ['f7244f5']
---

# Summary
Unified the widget architecture across National Rail, Tube, and Bus boards by decoupling the service list. Split the list into a static header `row0Widget` and a dynamic scrolling `servicesWidget`. Integrated the `weatherWidget` and `clockWidget` into the TfL and Bus headers to establish visual parity with the National Rail layout. Refactored layout interfaces (`iTflLayout`, `iBusLayout`) internally to cleanly inject DI `TimeManager` dependencies via `.cpp` constructors instead of header inlines. Verified UI geometric conformity using the WASM testing layout simulator.

## Technical Context
- [sessions.md](sessions.md)
- [task.md](task.md)
