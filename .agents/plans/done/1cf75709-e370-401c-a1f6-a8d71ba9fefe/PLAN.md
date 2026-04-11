---
name: "LGV Terminology Refactor & Documentation Audit"
description: "Systematically renamed all instances of \"View\" to \"Layout\" across the core display architecture and architecture documents (LGL instead of LGV) to align with the \"Display Designer\" conceptual model. C..."
created: "2026-03-22"
status: "DONE"
commits: ['418144a']
---

# Summary
Systematically renamed all instances of "View" to "Layout" across the core display architecture and architecture documents (LGL instead of LGV) to align with the "Display Designer" conceptual model. Conducted a comprehensive documentation audit across the entire `modules/displayManager` directory to standardize module headers and add Doxygen comments to all widget and board controllers.

## Key Decisions
- **Terminology Alignment**: Migrated from the generic `View` term to `Layout` across C++ code and Markdown documentation to better describe the visual arrangement of widgets on the board.
- **Physical Structure Renaming**: Renamed all `views/` directories to `layouts/` and updated corresponding include paths.
- **Contract Standardization**: Renamed `iBoardView` to `iBoardLayout` and all derived board-specific interfaces (e.g., `iNationalRailView` -> `iNationalRailLayout`).
- **House Style Assurance**: Guaranteed documentation standards by appending mandatory Doxygen headers for all `displayManager` methods and ensuring module descriptions.
- **Hardware Verification**: Successfully flashed and verified on ESP32, confirming stable board initialization and data rendering under the new terminology.

**Archive**: [.agents/plans/done/1cf75709-e370-401c-a1f6-a8d71ba9fefe/](.agents/plans/done/1cf75709-e370-401c-a1f6-a8d71ba9fefe/)

## Technical Context
- [sessions.md](sessions.md)
- [task.md](task.md)
