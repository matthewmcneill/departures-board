# Walkthrough - Automated Build Timestamp Sync

I have implemented a robust automation system to ensure that both the hardware OLED and the Web Portal always reflect the correct build date and time.

## Changes Made

### 1. Build System Automation
- **[NEW] [build_timestamp.py](scripts/build_timestamp.py)**: A new Python script that runs at the beginning of every build. It generates a header file `modules/systemManager/build_time.h` containing:
    - `BUILD_TIME`: A unique build serial in the format `ByyyyMMddHHmmss-hash[+mod]` (e.g., `B20260328110500-7a2b5c+mod`).
    - `BUILD_DATE_PRETTY`: A human-readable date (`YYYY-MM-DD`).
- **Git Visibility**: The script now automatically detects if there are uncommitted changes in your workspace. If any files are modified and not yet committed, it appends a `+mod` suffix to the serial. This provides an honest indicator that the build contains code not yet officially part of the referenced commit.
- **[MODIFY] [platformio.ini](platformio.ini)**: Registered the new script as a `pre` script for all environments.

### 2. Firmware Integration (C++)
- **[MODIFY] [systemManager.cpp](modules/systemManager/systemManager.cpp)**: Now includes the generated header. The `getBuildTime()` function returns the full build serial.
- **[MODIFY] [displayManager.cpp](modules/displayManager/displayManager.cpp)**: Updated to use the full `BUILD_TIME` serial on the loading screen footer. 

### 3. Web Portal Sync
- **[MODIFY] [portalBuilder.py](scripts/portalBuilder.py)**: Added a regex injection step to the asset pipeline. It now reads the `BUILD_TIME` serial from the generated C++ header and patches the web UI automatically to ensure 100% synchronization between firmware and portal.

## Verification Results

### Automated Build Test
- Ran `pio run` and verified that the build system successfully triggered both the header generation and the portal patching.
- Both `systemManager.cpp` and `displayManager.cpp` were correctly recompiled during the flash cycle.

### Hardware Validation
- Successfully flashed the device using a forced-kill on the serial monitor to free the port.
- **Loading Screen**: Displays the full `B...-hash` serial in the footer.
- **Web UI**: The System tab now shows the exact same serial, providing total traceability to the source code state.

> [!TIP]
> The addition of the Git hash to the serial means you can always identify exactly which commit a specific device is running, even if multiple builds are made per day.
