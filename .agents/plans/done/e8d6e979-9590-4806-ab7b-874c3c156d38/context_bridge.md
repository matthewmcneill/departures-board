# Context Bridge

## 📍 Current State & Focus
The repository is currently perfectly stable and compiling under a monolithic build architecture. We have just resolved a serious PlatformIO Library Dependency Finder (LDF) compilation isolation error by permanently deleting all redundant nested `library.json` files from within the `modules/` hierarchy (specifically the `fonts` folder bounds) and forcing global dependency orchestration uniquely through `platformio.ini`. We also resolved case-sensitivity build issues on the `.hpp` dependencies and restored `systemBoard` layout generation hooking.

## 🎯 Next Immediate Actions
- Await the next User request or feature ticket.
- When creating any new architectural modules, do *not* nest a `library.json` file inside its directory—allow `lib_extra_dirs` to scoop it implicitly.

## 🧠 Decisions & Designs
- **Monolithic Build Rule Enforcement**: `platformio.ini` operates as the single source of truth for all project configurations and external dependency resolutions.
- **Git Case Adherence**: Always rely on `git mv` when modifying capitalized files on macOS/Windows, as default `mv` operations bypass internal Git case tracking on APFS filesystems.

## 🐛 Active Quirks, Bugs & Discoveries
- The FreeRTOS PlatformIO SDK compiler leverages `lib_ldf_mode = deep+`. If a module folder natively possesses a `library.json` file inside it, LDF physically walls off that entire sub-directory and will not inject any `.cpp` files within unless the parent component explicitly declares a downstream dependency on it via JSON graph logic.

## 💻 Commands Reference
- Run test unit compiler logic: `pio test -e unit_testing_host`
- Trigger physical full firmware generation: `pio run`

## 🌿 Execution Environment
- Branch: `main`
- Hardware Lock `NONE` (Plan cleanly detached and finalized).
