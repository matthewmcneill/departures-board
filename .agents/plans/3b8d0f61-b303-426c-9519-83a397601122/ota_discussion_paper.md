# Over-The-Air (OTA) System Discussion Paper

## Overview
The OTA system in the `departures-board` project provides a hands-free firmware lifecycle mechanism. It is fundamentally an "Appliance" style updater. Rather than prompting the user via a UI to install an update (the "Mobile OS" approach), the device silently checks for a new build on GitHub and structurally commandeers the firmware on detection to download, flash, and restart.

## How It Works with GitHub
The process of synchronizing the firmware with GitHub is handled via three main components:

### 1. `githubClient`
The system utilizes a custom `githubClient` class to talk securely with `api.github.com`. It queries the endpoint `/repos/gadec-uk/departures-board/releases/latest` over HTTPS.
- **Memory Efficiency**: Since GitHub JSON payloads can be exceptionally large, it avoids allocating the entire payload into RAM using an `ArduinoJson` document block. Instead, it processes the HTTP stream character-by-character using a `JsonStreamingParser`.
- **Parsing Targets**: It searches for keys matching `"tag_name"` (to establish the remote version) and iterates through the `assets` arrays specifically hunting for an asset named `"firmware.bin"`.

### 2. `otaUpdater`
This is the master orchestrator wrapper (`lib/otaUpdater/otaUpdater.cpp`). 
- **Trigger**: Currently, `otaUpdater::tick()` debounces check intervals (approx every ~1 hour). If the current day of the month (`tm_mday`) hasn't been checked yet, it triggers an API poll. Effectively, it looks for an update on the first hour of a new day that the board registers.
- **Version Checking**: The `isFirmwareUpdateAvailable()` method compares the local `VERSION_MAJOR` and `VERSION_MINOR` constants against the extracted `"tag_name"`. 
- **UI Interaction**: Upon discovering an update, the updater issues a 30-second callback countdown (`onWarning`). Historically, this drove the `FirmwareUpdateBoard` UI takeover so the user knew not to yank the power cord. 

### 3. `HTTPUpdateGitHub`
This handles the actual binary flashing (`lib/HTTPUpdateGitHub`).
- It initiates a secure download stream of the `firmware.bin` from GitHub.
- It parses headers safely and directs the payload into the inactive OTA partition (`U_FLASH`) on the ESP32 using the intrinsic `Update` library.
- It supports authentication via access tokens for private repository pulling, and elegantly follows GitHub's redirect architecture (302 found) when navigating to actual AWS binary asset storage buckets.

## Addressing the Requested Features

With the system's baseline architecture understood, here are considerations for implementing your requests:

### 1. Daily Auto-check at a Specified Quiet Hour
**Current State**: The check triggers once a day arbitrarily dependent on the hour the device was turned on (it executes on the very first hour registered on a new calendar day).
**Proposed Change**: 
- Introduce a new variable like `otaQuietHour` (0-23) within the `Config` struct and expose it via the dashboard. 
- Modify `otaUpdater::tick()` to only execute the daily GitHub fetch if `timeinfo.tm_hour == cfg.otaQuietHour` AND `timeinfo.tm_mday != prevUpdateCheckDay`. 
- This guarantees updates cleanly occur during non-viewing periods (e.g., 3 A.M.).

### 2. Function to Check Whether an Update Exists (Return True/False)
**Current State**: The check logic currently runs entirely internally and directly consumes the update if one is found.
**Proposed Change**: 
- Extract the probing logic into a dedicated, callable function within `otaUpdater` (e.g., `bool checkUpdateAvailable(String& availableVersion)`).
- This will instantiate the `ghUpdate.getLatestRelease()` process.
- Can be exposed to a new background task or the Web UI for a "Check for Updates" polling sequence, allowing you to optionally flag a silent widget or report via API.

### 3. Function to Force an Update Now
**Current State**: A function `checkForFirmwareUpdate()` actually already exists in `otaUpdater.cpp`, which forces the download *if* `isFirmwareUpdateAvailable()` evaluates to true.
**Proposed Change**:
- Allow UI integration (via `webHandlerManager`) to trigger `ota.checkForFirmwareUpdate()` manually on demand.
- We might want to add an override method (like `forceUpdateNow(bool ignoreVersionCheck)`) should you wish to manually re-flash the system bypassing the semantic version safety gates.

---
Let me know if you would like me to draft an implementation plan encompassing these three changes.
