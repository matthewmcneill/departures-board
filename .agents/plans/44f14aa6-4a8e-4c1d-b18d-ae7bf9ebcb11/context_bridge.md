# Context Bridge

**📍 Current State & Focus**
The attribution constants (`nrAttributionn`, `tflAttribution`, `btAttribution`) were successfully refactored out of the global scope in `src/departuresBoard.cpp` and localized into their respective board implementations (`nationalRailBoard.cpp`, `tflBoard.cpp`, `busBoard.cpp`). This removes minor architectural debt and strictly encapsulates the proprietary text inside the correct domain controllers.

**🎯 Next Immediate Actions**
No immediate next actions for this specific refactor as it is completed and contained. Provide evaluation or testing of this codebase if desired.

**🧠 Decisions & Designs**
Relocated the constants to strictly localize them within each board implementation, eliminating `extern` global floating constants for display features from the system's root scope.

**🐛 Active Quirks, Bugs & Discoveries**
None currently tracked in relation to this change.

**💻 Commands Reference**
Standard PlatformIO build and flash commands remain relevant.

**🌿 Execution Environment**
Working on a MacOS machine in `departures-board`, no apparent hardware connected context in active use in this session.
