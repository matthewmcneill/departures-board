# Automated Build Timestamp Implementation Plan

This plan addresses the issue where the firmware and web UI show outdated build dates. It introduces a reliable, automated mechanism to synchronize the build timestamp across all interfaces.

[x] Reviewed by architectural-refactoring - passed
[x] Reviewed by embedded-systems - passed (minimal heap/flash impact)

## User Review Required

> [!IMPORTANT]
> The build system will now generate a temporary header file `modules/systemManager/build_time.h` on every build. This ensures the OLED loading screen and Web UI always show the actual build serial in the format `ByyyyMMddHHmmss-hash` (e.g., `B20260328110500-7a2b5c`).

## Proposed Changes

### Build System & Scripts

---

#### [NEW] [build_timestamp.py](scripts/build_timestamp.py)
A new PlatformIO pre-script to generate the C++ build time header.

#### [MODIFY] [portalBuilder.py](scripts/portalBuilder.py)
Update to dynamically replace the hardcoded date in `web/index.html` during the asset generation phase.

#### [MODIFY] [platformio.ini](platformio.ini)
Register the new `build_timestamp.py` script.

### Firmware (C++)

---

#### [MODIFY] [systemManager.cpp](modules/systemManager/systemManager.cpp)
Include the generated header and use the new macro for `getBuildTime()`.

#### [MODIFY] [displayManager.cpp](modules/displayManager/displayManager.cpp)
Include the generated header and use the macro for the loading screen build time.

## Open Questions

- None at this stage. The approach leverages existing PlatformIO script patterns.

## Verification Plan

### Automated Tests
- Run `pio run` and verify `modules/systemManager/build_time.h` is created with the current time.
- Verify `modules/webServer/portalAssets.h` contains the updated date in its gzipped stream (indirectly via manual check or script log).

### Manual Verification
- Check the OLED loading screen after a fresh flash.
- Check the System tab in the Web UI.
