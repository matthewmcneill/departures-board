# Context Bridge

**📍 Current State & Focus**
The project is currently refactoring out hardcoded UI strings used for data API attributions into a polymorphic architecture driven by the `iDataSource` boundary. The implementation plan has been designed and approved by the user, and the first task execution was underway: `virtual getAttributionString()` has been added to `iDataSource.hpp`.

**🎯 Next Immediate Actions**
Resume execution of the `/task.md` steps by opening up `modules/displayManager/boards/nationalRailBoard/nrDARWINDataProvider.hpp` and `.cpp` and implementing `getAttributionString()`, repeating this for all data providers sequentially.

**🧠 Decisions & Designs**
The user directed us to push the new Weather & RSS attributions natively into the `globalMessagePool` once on boot, rather than append them during UI transitions.

**🐛 Active Quirks, Bugs & Discoveries**
`iDataSource.hpp` default implementations for `getAttributionString()` are intentionally `nullptr`, meaning no changes need to break compilation bounds for arbitrary mock objects unless strictly required. 

**💻 Commands Reference**
Native automated test build: `pio run -e native`
For validation later: `.agents/skills/hardware-testing/scripts/safe-flash.sh`

**🌿 Execution Environment**
Working on the `main` or active refactor branch. The hardware simulator UI testing workflow is actively running.
