# Lazy-Loading System Boards

- [x] Modify `DisplayManager::begin()` to remove upfront static allocations.
- [x] Implement lazy-loading logic in `DisplayManager::getSystemBoard(id)`.
    - [x] `SYS_BOOT_SPLASH` (SplashBoard)
    - [x] `SYS_BOOT_LOADING` (LoadingBoard)
    - [x] `SYS_WIFI_WIZARD` (WizardBoard)
    - [x] `SYS_SETUP_HELP` (HelpBoard)
    - [x] `SYS_FIRMWARE_UPDATE` (FirmwareUpdateBoard)
    - [x] `SYS_SLEEP_CLOCK` (SleepingBoard)
    - [x] `SYS_DIAGNOSTIC` (DiagnosticBoard)
    - [x] Shared `MessageBoard` for all `SYS_ERROR_*` IDs.
- [x] Verify boot animation still functions as expected.
- [x] Verify clean PlatformIO build.
