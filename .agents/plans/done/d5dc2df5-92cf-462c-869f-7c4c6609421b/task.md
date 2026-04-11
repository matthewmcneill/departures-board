# House Style Documentation Pass

- [x] Load the `@.agents/skills/house-style-docs` skill and review syntax requirement.
- [x] Review documentation in core modules:
    - [x] `modules/webServer/webServer.hpp` (Needs manual update for header exports)
    - [x] `modules/configManager/configManager.hpp`
    - [x] `src/departuresBoard.cpp`
- [x] Create expanded implementation plan to outline changes.
- [x] Receive user approval of expanded implementation plan.
- [/] Execute global compliance sweep:
    - [x] Update header blocks in 8 System Board files.
    - [x] Update header blocks in 10 Widget files.
    - [x] Update header blocks in 7 remaining files (Data Manager, Message Pool, Web Server, etc.).
    - [x] Perform manual touch-ups on `departuresBoard.cpp`.
- [x] Verify compliance:
    - [x] Re-run `check_compliance.py` (ensure 0 issues for manual files).
    - [x] Run `pio run -e esp32dev` to confirm no breakage.
