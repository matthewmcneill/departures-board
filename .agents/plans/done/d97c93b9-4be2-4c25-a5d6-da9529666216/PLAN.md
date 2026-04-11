---
name: "Hardware Pagination Fix & Negative Overflow Prevention"
description: "Resolved an unintended state-machine wipeout occurring during the 4000ms `nextViaToggle` loops that blocked the 5000ms `serviceListWidget` from ever physically scrolling multi-page data on National Ra..."
created: "2026-04-03"
status: "DONE"
commits: ['7790ace']
---

# Summary
Resolved an unintended state-machine wipeout occurring during the 4000ms `nextViaToggle` loops that blocked the 5000ms `serviceListWidget` from ever physically scrolling multi-page data on National Rail boards. Stabilized the XML streaming parser by enforcing `indexOf` strict substring validations and fixing a critical integer underflow crash `(id < unsigned)` caused by negative initialization.

## Technical Context
- [sessions.md](sessions.md)
- [context_bridge.md](context_bridge.md)
- [task.md](task.md)
