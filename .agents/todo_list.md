# Next Session TODO

### Web Portal & UI
- [ ] Fix 'Rail Station' field population to resolve CRS to human-readable names on load (currently empty until manual input).
- [ ] Investigate missing Latitude/Longitude fields in the 'Advanced Options' tab of the Web UI.
- [ ] Add unit tests for `ConfigManager::hasConfiguredBoards()` to verify it correctly evaluates incomplete default boards as unconfigured, preventing the system from skipping the `BOARD_SETUP` sequence.
- [ ] departuresBoard.hpp - does it have all the platformio.ini variables in it - what about max keys?  (btw we probably only need 4 max - 6 to be generous)
- [ ] add a machine state to app context for OTA updates. If the ota update flag is checked it should go to UPDATING state before running.  In this state it should check for updates, and then do an OTA if there is one whilst displaying the OTA wizard or progress bar. Otherwise it just checks and then moves to RUNNING.  We also need to think about how a long running device periodically goes to UPDATING state to check.
- [/] need to review all password masking. (Stage 1: Implemented logic to prevent overwriting keys/passwords if the placeholder is unchanged. Stage 2: Refining grey stars indicator UI).
- [ ] add an 'X' close button to the top right of the keys detailed form (acts as cancel).
- [ ] ensure API key save commits existing values if no changes were made.
- [ ] check if the key UI provides format validation when keys are entered.
- [ ] include instructions and links on where to get keys in the key creation UI (replicate help strings from `web/keys.htm`).
- [ ] Add tooltips or help text for "No Scrolling" and "Fast Refresh" settings in the System tab.
- [ ] consider how we add validation to the filter strings to the portal to ensure that the filters make sense.
- [ ] **Diagnostics**: Add real-time OLED power state (On/Off) to the web portal's hardware status section.
- [ ] add the max heap size to the portal system page for hardware status section.
- [ ] **Web**: Consider adding a "Credits" tab to the web portal with attributions to authors (Gadec Software, Matt McNeill) and data providers (National Rail, TfL, bustimes.org). Reference `nrAttribution[]`, `tflAttribution[]`, and `btAttribution[]` strings.

### Firmware & Architecture
- [ ] **Config**: Add the constraints from `.agents/rules/explicit-execution.md` to the Antigravity extension's 'Workspace Custom Rules' to permanently enforce the lock/queue authorization structure.
- [ ] departuresBoard.hpp - does it have all the platformio.ini variables in it - what about max keys?  (btw we probably only need 4 max - 6 to be generous)
- [ ] add a machine state to app context for OTA updates. If the ota update flag is checked it should go to UPDATING state before running.  In this state it should check for updates, and then do an OTA if there is one whilst displaying the OTA wizard or progress bar. Otherwise it just checks and then moves to RUNNING.  We also need to think about how a long running device periodically goes to UPDATING state to check.
- [ ] make an ota widget and remove the OTA overlay board
- [ ] Migrate Sleep Schedule from v1 config to a unified 'Schedule' tab/module (currently handled in legacy System screens).
- [ ] Investigate moving the "Refresh Rate" configuration from a global system setting to a per-key property.
- [ ] **Schedule Migration**: Review the original v1/v2 `config.json` format. Critically assess how existing `sleepEnabled`, `sleepStarts`, and `sleepEnds` fields should be automatically migrated into the new Schedule/Screensaver paradigm when we update the settings json parsing logic.

### Data Providers & Scheduling
- [ ] header widgets - I have a feeling this is not generalised, and is spacific to NatRail. (Still largely NR specific)
- [ ] Add configuration option to set custom timeout durations for RSS feeds based on feed size, bypassing the hardcoded 10s default.
- [ ] have the iDataProviders give a data-expiry time so that it can tell the scheduler that it should update (for example just after the last train was expected to depart) or do it at least once every minute.

### Diagnostics & Testing
- [ ] Investigate and fix `pio test` native compilation errors. Currently, the Arduino framework test environment lacks a structural `setup`/`loop` entry point causing `undefined reference` linker errors.
- [ ] Add unit tests for `ConfigManager::hasConfiguredBoards()` to verify it correctly evaluates incomplete default boards as unconfigured, preventing the system from skipping the `BOARD_SETUP` sequence.
- [ ] Add diagnostic UI to expose real-time FreeRTOS TWDT (Watchdog) metrics and background task states for each data fetcher.
- [ ] Implement C++ unit testing for the new `dataManager` module to verify priority queue behavior on connection timeouts and duplicate fetch requests.
- [ ] Implement a real-time 'Background Polling Log' in the web portal diagnostic UI for visibility into distributed update ticks.
- [ ] add a test class to `iDataSource` (e.g., `MockDataSource`) to keep data source code encapsulated and improve unit testability.
- [ ] Add unit test or integration scenario for `WebServer::handleClient()` concurrent API requests during long data fetches to prevent re-entrancy bugs.
- [ ] Implement C++ unit tests for `TimeManager` to verify NTP sync logic and custom timezone offset calculations now that it is decoupled from the hardware globals.
- [ ] Audit `temperatureRead()` accuracy across ESP32 chip revisions (V1 vs V3) for more reliable diagnostic reporting.
- [ ] Add unit test or manual verification suite for `determineInitialTab` across all state transitions to ensure long-term stability as more tabs are added.
- [ ] **Unit Testing**: Implement `unit_testing_host` scenarios for `drawText` to verify fast-path logic and rough-cut truncation accuracy under extreme string lengths.
- [ ] **Unit Testing**: Implement `ConfigManager` migration scenarios to verify v2.3 -> v2.4 legacy `turnOffOledInSleep` to per-board `oledOff` mapping.

### Simulator Parity
- [ ] **Simulator Parity**: Update the WebAssembly `layoutsim` mock data injection (`syncData()` in `main.cpp`) to accurately populate the `location` memory array rather than defaulting to `stationName`, preventing false-blank simulator displays for National Rail layouts.
- [ ] **Simulator**: Update `layoutsim` to handle the `oledOff` property and simulate display blanking in the browser.
- [ ] **Simulator**: in the WASM if the edited json omits a widget config mark it as `isVisible = false` and do not show it. (makes config show only configured widgets)

### Uncategorized / Other

## Completed
- [x] startup screens don't show and then the board display seems very static and is not laid out properly. need to work through this. Probably starting with the startup screens and then working through the system screens. (Resolved: Added AppState::RUNNING guards to display scheduler and data fetchers to fix boot-time regression)
- [x] ~~Add unit testing for the SystemManager Round-Robin update multiplexing queue to monitor data load distributions across high board counts.~~ (OBSOLETE: `SystemManager` God Object was dismantled and responsibilities migrated to `appContext` and domain managers)
- [x] review the whole data request queueing onto the other core for async - how we get queed - how to jump priority, how to update at the right time.
- [x] **OLED Sleep Refactor**: Migrated "Turn OLED Off" from global system settings to per-board `BoardConfig`. Implemented v2.4 JSON migration and state-tracked power management in `DisplayManager`.
- [x] Implement a unified diagnostic test queue in the frontend to prevent ESP32 resource saturation and dropped connections.
- [x] Unify the "Active Displays" diagnostic UI with the "API Keys" style (added status text labels next to dots).
- [x] Auto-abort pending diagnostic tests when switching tabs to focus on the current context.
- [x] move all the u8g2 setup and handling into the display manager. Add a displaymanager.Showboard( iDisplayBoard ) instead of calling render directly whcih injects the display hardware. Move all of the constants related to the display into the display manager
- [x] clean up the drawing primitives. These routines should all take a u82g display as a paramaeter and not assume the global one exists. (Consolidated into unified drawText API in session bd5ae513)
- [x] Refactor Transport Boards (TfL, Bus, NR) to use the new `labelWidget` for empty/error states instead of raw drawing primitives.
- [x] continue with the rationalisation of the data providers
- [x] finish the updates to the web configuration front end to enable better board configuration (Overhauled Displays tab: Startup highlight, reorder buttons, dynamic instructions, and smart initial tab selection)
- [x] iStation and messageData - shoudl be obsolete
- [x] systemBoard.cpp still has a lot of old straggling stuff that needs to go into displays, widgets or modules. (Cleanup complete, broken into separate classes)
- [x] Departures Board (systemManager) still has a bunch of specific data to the various boards which needs moving into the classes.
- [x] check that the active display boards are maintained correctly when going through sub-loops of the main loop (e.g. OTA check flow handles this)
- [x] there is a weird dependency on the fonts in the widgets - to calculate font size we set properties on the u8g2 board and pull a value which may upset actual board state.
- [x] rationalise the include folder and get rid of it. (Renamed libraries and moved assets, legacy code removal queued/completed in session 2ec805fd)
- [x] House Style Refactor: libraries to camelCase and headers documented. (Completed: Audited repository, added Doxygen, renamed headers to .hpp)
- [x] why do we haev an appContext class, it seems to just to add one layer of dereference from the departureboard.cpp. Is it superflouous? (Resolved: Clarified DI orchestration role and encapsulated in modules/appContext).
- [x] need to check that the config is being upgraded and then rewritten - looks like the old one. (Rewritten to modernize on save and serve from memory)
- [x] check that when dumping API keys we have already registered them as secrets in the logger (implemented in appContext.cpp)
- [x] Port existing functionality: WiFi Reset (backend & frontend).
- [x] Investigate and fix intermittent synchronization issues/delays in the Web GUI mode switching (Resolved: Implemented sequential validation to prevent ESP32 saturation).
- [x] Add RSS feed configuration UI to the System tab (Implemented in FEEDS tab).
- [x] the 'eye' visible icon for the passwords is stil misaligned - I asked the agent 3 times to fix it and it just could not understand. (Fixed: Logic corrected and alignment verified on hardware)
- [x] the scan button is not aligned with the networks drop-down box and changes size when the scan icon is showing. (Verified: Stable alignment and sizing)
- [x] just check that the key registry IS actually being used as a registry. i.e. anything that references it uses an ID for the key that is served by the registry with the actual key. Similarly, in the storage, the configuration storage should reference the id of the key and the apiKeyManager should use that ID to serve the correct key. (Verified: IDs correctly used across board mapping and save logic)
- [x] need to add in the footers containing credits and acknowledgements to the bottom of the portal screen (Implemented in all tabs)
- [x] Investigate persistent National Rail SOAP connection instability (success on WSDL boot, but frequent "SOAP connection failed" during data updates; requires retry logic evaluation). (Resolved: Implemented FreeRTOS asynchronous yielding).
- [x] Refactor data sources (weather, national rail, tfl, bus) to use a Centralized Worker Queue, preventing `WiFiClientSecure` OOM exceptions.
- [x] need to fix the national rail key and testing process in the portal. Currently it always fails. (Resolved: Implemented sequential testing to handle connection overhead).
- [x] fix the 'Testing' button height in the keys detailed form to ensure it remains constant when text changes. (Resolved: Buttons now have consistent vertical rhythm in modern modal layout).
- [x] remove the bus key button from the add keys dialog. No key is needed for bus times. (Resolved: Bustimes.org uses scrapers/unauthenticated APIs).
- [x] move the host-name back to wifi config. It will help the user to find the device on the network once the wifi is configured. It doesn't need to be in system settings.
