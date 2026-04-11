---
name: "Clock Widget Extension & WASM Schema Validation"
description: "Implemented support for the `HH_MM_SS` format and custom secondary font allocations in the `clockWidget`. Architected a seamless C++ to WASM registry mapping mechanism that enables the layout simulato..."
created: "2026-03-28"
status: "DONE"
commits: ['388b36f']
---

# Summary
Implemented support for the `HH_MM_SS` format and custom secondary font allocations in the `clockWidget`. Architected a seamless C++ to WASM registry mapping mechanism that enables the layout simulator to introspect C++ types.

## Key Decisions
- **U8g2 Baseline Alignment**: Implemented font-safe `getAscent()` arithmetic to vertically match the seconds digits directly to the primary hours/minutes baseline, avoiding hardcoded coordinate offsets.
- **Secondary Font Hydration**: Decoupled the structural representation of the widget's "seconds" from its font size, enabling layouts to specify `"format": "HH_MM_SS"` and an optional `"secondaryFont"`.
- **WASM Schema Introspection**: Refactored `gen_sim_registry.py` and `DesignerRegistry` to embed C++ types during compilation, ensuring the simulator UI models unmapped components with fully populated default JSON properties on-click.
- **Blink State Consolidation**: Disabled the continuous colon 500ms flash toggle on the seconds separator, bringing it in line with official National Rail specifications where only the HH::MM colon flashes.

**Archive**: [.agents/plans/done/2c2d6708-ef88-405e-bc90-0281c833e573/](.agents/plans/done/2c2d6708-ef88-405e-bc90-0281c833e573/)

## Technical Context
- [sessions.md](sessions.md)
- [task.md](task.md)
