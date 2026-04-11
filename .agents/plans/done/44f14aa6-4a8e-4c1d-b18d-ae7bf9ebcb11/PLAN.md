---
name: "Relocating Attribution Constants"
description: "Refactored `nrAttribution`, `tflAttribution`, and `btAttribution` out of the global scope in `src/departuresBoard.cpp` into their respective board implementations (`nationalRailBoard.cpp`, `tflBoard.c..."
created: "2026-04-02"
status: "DONE"
commits: ['a89c7e3']
---

# Summary
Refactored `nrAttribution`, `tflAttribution`, and `btAttribution` out of the global scope in `src/departuresBoard.cpp` into their respective board implementations (`nationalRailBoard.cpp`, `tflBoard.cpp`, `busBoard.cpp`). This removes minor architectural debt and strictly encapsulates display-only text within the correct domain controllers.

## Technical Context
- [sessions.md](sessions.md)
- [context_bridge.md](context_bridge.md)
