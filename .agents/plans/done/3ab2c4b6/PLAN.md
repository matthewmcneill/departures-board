---
name: "labelWidget UI Component Architecture"
description: "Implemented a new graphical component, `labelWidget`, to wrap bare-metal `u8g2` drawing primitives in an object-oriented state. This abstraction was subsequently deployed across all Transport-class Bo..."
created: "2026-03-22"
status: "DONE"
commits: ['9f6445e']
---

# Summary
Implemented a new graphical component, `labelWidget`, to wrap bare-metal `u8g2` drawing primitives in an object-oriented state. This abstraction was subsequently deployed across all Transport-class Boards (`TfL`, `Bus`, `NationalRail`) by introducing an architectural pattern of "Visibility Toggling" during missing-data states rather than dynamic imperative drawing routines.

## Key Decisions
- **Designer Preparedness**: Implemented `labelWidget` inheriting `iGfxWidget` and explicitly tagged `setFont`, `setAlignment`, and `setTruncated` setters with `@designer_prop` to guarantee forwards compatibility with the upcoming visual drag-and-drop Display Designer tool.
- **Embedded SRAM Policy**: Intentionally excluded the static "System Boards" (`splashBoard`, `loadingBoard`, `messageBoard`, etc.) from adopting `labelWidget`. This preserves roughly 140 bytes of permanent SRAM per string that would otherwise be wasted encapsulating strings that never dynamically update or move. Documented this deliberate non-migration within `docs/SystemSpecificationDocument.md`.
- **Visibility-Driven Layouts**: The empty data fallback behavior ("No scheduled services.") on Transport Boards was migrated directly into the base Layout classes (`iTflLayout`, `iBusLayout`, etc.) using `labelWidget` objects that are toggled on and off via `.setVisible()`.
- **Validation**: Performed sequential `esp32dev` and `esp32s3nano` PlatformIO builds to ensure full native compilation compatibility and no static memory bloat.

## Technical Context
- [sessions.md](sessions.md)
