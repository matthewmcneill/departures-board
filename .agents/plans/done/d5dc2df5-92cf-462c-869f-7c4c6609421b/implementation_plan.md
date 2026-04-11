# Goal Description

Perform a comprehensive House Style Documentation pass across the `departures-board` project, specifically targeting the core managers, to ensure 100% compliance with the `house-style-docs` standards without altering any underlying business logic.

## User Review Required

There are no breaking changes or critical logic modifications. This relies purely on commenting non-compliant variables and function declarations and adding the missing `Exported Functions/Classes:` header block to relevant files.

> [!NOTE]
> Auto-generated files (e.g., `layout*.hpp/cpp`, `fonts.cpp/hpp`) are explicitly ignored in this pass as they are managed by build-time generator scripts.

## Proposed Changes

### Core Managers & Services (Previously Defined)

#### [MODIFY] [webServer.hpp](modules/webServer/webServer.hpp)
- Add parameter docstrings and handler manager description.
- Add missing `Exported Functions/Classes:` header block.

#### [MODIFY] [configManager.hpp](modules/configManager/configManager.hpp)
- Fix reloadPending inline doc style and ScheduleRule comments.
- Add brief and return to reload functions.

#### [MODIFY] [departuresBoard.cpp](src/departuresBoard.cpp)
- Add a same-line comment explanation on initialization of `appContext appContext;` 

---

### Global Compliance Sweep (Missing 'Exported Functions/Classes:' header)

The following files will be updated to include the missing `Exported Functions/Classes:` block in their module headers. For `.cpp` files, this will typically be marked as `None` (referencing the `.hpp` for exports) or listing internal classes/functions if they are local to the translation unit.

#### [MODIFY] System Boards (modules/displayManager/boards/systemBoard/)
- `firmwareUpdateBoard.cpp`
- `loadingBoard.cpp`
- `diagnosticBoard.cpp` & `diagnosticBoard.hpp`
- `helpBoard.cpp`
- `sleepingBoard.cpp`
- `wizardBoard.cpp`
- `splashBoard.cpp`
- `messageBoard.cpp`

#### [MODIFY] Widgets (modules/displayManager/widgets/)
- `weatherWidget.cpp` & `weatherWidget.hpp`
- `locationAndFiltersWidget.cpp`
- `scrollingTextWidget.cpp`
- `progressBarWidget.cpp`
- `scrollingMessagePoolWidget.cpp`
- `serviceListWidget.cpp`
- `drawingPrimitives.cpp`
- `systemMessageWidget.cpp`
- `imageWidget.cpp`
- `labelWidget.cpp`

#### [MODIFY] Others
- `modules/displayManager/boards/nationalRailBoard/iNationalRailLayout.cpp`
- `modules/displayManager/messaging/messagePool.cpp`
- `modules/dataManager/dataManager.cpp`
- `modules/webServer/webServer.cpp`
- `modules/webServer/portalAssets.cpp`

---

## Verification Plan

### Automated Tests
- Run PlatformIO build command to verify that no functional logic was broken and that the code compiles cleanly: `pio run -e esp32dev`.
- Re-run the compliance script `check_compliance.py` to ensure zero non-generated files remain on the list.

### Manual Verification
- Review the diff generated across the modified files to ensure comment format strictly aligns to the `@.agents/skills/house-style-docs/SKILL.md` template.
