---
name: "Portal Navigation Reorder & Build Script Fix"
description: "Reordered the bottom navigation bar of the web portal to follow the user flow (WiFi, Keys, Feeds, Displays, Schedule, System). Added a placeholder for the \"Schedule\" feature and fixed a build script i..."
created: "2026-03-17"
status: "DONE"
commits: ['ba084c4']
---

# Summary
Reordered the bottom navigation bar of the web portal to follow the user flow (WiFi, Keys, Feeds, Displays, Schedule, System). Added a placeholder for the "Schedule" feature and fixed a build script integration issue that was preventing the portal assets from being correctly embedded during compilation.

## Key Decisions
- **User Flow Reorder**: Moved the navigation links and their corresponding content sections in `index.html` to align with the logical setup sequence requested.
- **Schedule Tab Placeholder**: Added a new "Schedule & Auto-Power" tab with a "Feature under development" message and icon, providing a UI hook for future development.
- **`portalBuilder.py` Integration**: Corrected a logic error in the build script that only ran the asset generation if the script was executed as `__main__`. This fix ensures it runs correctly when invoked by PlatformIO as a `pre:` script, guaranteeing the latest portal changes are included in every firmware build.
- **Serial Port Cleanup**: Implemented aggressive process termination for stale `pio` and `esptool` instances to resolve hardware upload conflicts.

## Technical Context
