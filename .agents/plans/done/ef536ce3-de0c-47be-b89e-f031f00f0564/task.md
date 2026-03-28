## Tasks - Automated Build Timestamp Sync

- `[x]` Create `scripts/build_timestamp.py` to generate `modules/systemManager/build_time.h`
- `[x]` Register `scripts/build_timestamp.py` in `platformio.ini`
- `[x]` Update `modules/systemManager/systemManager.cpp` to use `BUILD_TIME` macro
- `[x]` Update `modules/displayManager/displayManager.cpp` to use `BUILD_TIME` macro
- `[x]` Update `scripts/portalBuilder.py` to patch `web/index.html` with refined build serial
- `[x]` Refined build serial to `ByyyyMMddHHmmss-[hash]` format
- `[x]` Added 'dirty' tree detection (`+mod` suffix for uncommitted changes)
- `[x]` Verify with `pio run`
- `[x]` Check OLED loading screen and Web UI (Requires flashing)
