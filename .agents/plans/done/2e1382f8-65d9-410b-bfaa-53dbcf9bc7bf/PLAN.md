---
name: "TfL & Bus Board Memory Safety & Layout Upgrade"
description: "Successfully resolved a critical memory corruption issue in the TfL and Bus board firmware by refactoring data structures to a zero-copy architecture. Upgraded both boards to a high-fidelity 4-column ..."
created: "2026-03-28"
status: "DONE"
commits: ['10e5e55']
---

# Summary
Successfully resolved a critical memory corruption issue in the TfL and Bus board firmware by refactoring data structures to a zero-copy architecture. Upgraded both boards to a high-fidelity 4-column layout (Order, Line/Route, Destination, Time) to match London transport standards. Replaced dangerous stack-allocated string pointers with static, persistent pointers for service numbering.

## Key Decisions
- **Zero-Copy Numbering**: Implemented a `static const char*` array (`serviceNumbers`) in `TflDataSource` and `BusDataSource`. This provides persistent, zero-overhead pointers for service position numbers ("1" through "20"), eliminating dangling pointers in the `serviceListWidget`.
- **High-Fidelity Layouts**: Updated `layoutDefault.json` and `layoutDefault.cpp` for both boards to a 4-column structure. Refactored `updateData()` in both board controllers to pass the `orderNum` pointer to the `serviceListWidget` conditionally based on the `config.showServiceOrdinals` setting.
- **Architectural Cleanup**: Fixed a syntax error in `modules/schedulerManager/schedulerManager.cpp` (missing `if` condition for change detection) that was blocking compilation and corrected loop scope issues in `TfLBoard::updateData()`.
- **House Style Alignment**: Audited and updated all modified modules to strictly adhere to v3.0 project standards, including Doxygen headers and step-by-step functional comments.

**Archive**: [.agents/plans/done/2e1382f8-65d9-410b-bfaa-53dbcf9bc7bf/](.agents/plans/done/2e1382f8-65d9-410b-bfaa-53dbcf9bc7bf/)

## Technical Context
- [sessions.md](sessions.md)
- [context_bridge.md](context_bridge.md)
