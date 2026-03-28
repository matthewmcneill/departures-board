# Task: Evaluate Archiving Old Web Infrastructure

- [x] Identify all files in "old" web infrastructure <!-- id: 0 -->
     [x] Identify legacy routes and includes in `webServer.cpp`
    [x] Assess OTA updater dependencies on legacy headers
    [ ] Move and unify version constants in `departuresBoard.hpp`
    [ ] Plan stubs for legacy OTA interactions in `otaUpdater.cpp`
- [x] Determine how old infrastructure is integrated <!-- id: 2 -->
    - [x] Check `platformio.ini` for build hooks <!-- id: 3 -->
    - [x] Check `scripts/` for web builders <!-- id: 4 -->
    - [x] Search for inclusions of headers in `include/webgui/` <!-- id: 5 -->
- [x] Evaluate feasibility of removal <!-- id: 6 -->
    - [x] Identify dependencies on old web infrastructure <!-- id: 7 -->
    - [x] Propose archive structure <!-- id: 8 -->
- [/] Update implementation plan for archive and rename <!-- id: 9 -->
    - [ ] Plan rename of `portal/` to `web/` <!-- id: 10 -->
    - [ ] Plan route change in `webHandlerManager.cpp` <!-- id: 11 -->
    - [ ] Plan test updates <!-- id: 12 -->
