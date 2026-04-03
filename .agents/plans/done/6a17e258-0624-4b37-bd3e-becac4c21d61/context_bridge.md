# Context Bridge

**📍 Current State & Focus**
We are in the midst of refactoring the configuration architecture in the `departures-board` project. We have successfully audited the codebase and formulated an `implementation_plan.md` to migrate JSON schema variables from `v2.4` to `v2.5` on the physical ESP32 flash.
The overarching focus is to eliminate the legacy sleep parameters from the `Config` hierarchy, forcefully deferring all UI time-gating fully to the native `SchedulerManager` module, and properly bridging OLED-off states. Concurrently, we are patching missing configuration inputs into the `web/index.html` frontend, such as a Daily OTA toggle and exposing the hidden National Rail specific `timeOffset` variable inside the UI correctly.
All workflows and `house-style` compliance audits were successful.

**🎯 Next Immediate Actions**
1. Read the `implementation_plan.md` in this directory.
2. Initialize a local `task.md` representation of the plan to structure execution sequences.
3. Migrate `departuresBoard.hpp`, `webHandlerManager.cpp`, `configManager.cpp`, and `configManager.hpp` according to the backend instructions.
4. Modify JavaScript in `web/index.html` to consume dynamically exported C++ bounds (like `MAX_SCHEDULE_RULES`) and render the new inputs accordingly.

**🧠 Decisions & Designs**
- Hardware geo-location coordinates (`lat`, `lon`, `weather`) are correctly maintained individually in `BoardConfig` configurations (per-board), accommodating stations spanning macro-climatic boundaries.
- `Time Offset` uses National Rail's Darwin precise WSDL spoof offsets. Because it doesn't translate to Bus or TfL APIs natively, we are leaving it as an "Advanced Option" explicitly restricted for the NR Board inside the Web UI. We explicitly decided against propagating it to a global variable.
- Configuration bounds (`MAX_BOARDS`, `MAX_SCHEDULE_RULES`) will be compiled natively from `#define` macros and exported explicitly through the JSON `/api/config` REST payload, overriding JavaScript `const` definitions. This builds a robustly extensible platform without desynchronizing the JS repo logic from the C++ hardware capabilities.

**🐛 Active Quirks, Bugs & Discoveries**
- The Darwin SOAP API directly anticipates the `<timeOffset>` SOAP parameter tags.
- Tube Board weather UI is currently locked out completely via Javascript inside `index.html`. The `t!==1` hiding code must be surgically removed.

**💻 Commands Reference**
Build/Upload: `./.agents/skills/hardware-testing/scripts/safe-flash.sh`
Or: `platformio run -e esp32dev`

**🌿 Execution Environment**
Standard MacOS with hardware attached physically for firmware validation. 

**⚠️ PORTABILITY RULE**
Repository-relative file structures strictly observed.
