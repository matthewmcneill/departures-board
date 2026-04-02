# Context Bridge

**📍 Current State & Focus**
Following the successful localization of attribution constants (`nrAttribution`, `tflAttribution`, `btAttribution`) into their respective board controllers, the focus has shifted to improving their visibility in the web portal. A new task has been added to the project todo list to implement a dedicated "Credits" tab that formally acknowledges data providers and authors.

**🎯 Next Immediate Actions**
1. Draft an implementation plan for the "Credits" tab in the web portal (`web/index.html`).
2. Design the UI layout for the Credits tab, ensuring it matches the existing portal's aesthetic (Gadec Software style).
3. Identify all necessary attributions, including the Creative Commons license details.

**🧠 Decisions & Designs**
- **Dedicated Credits Tab**: Decided to transition from simple footer acknowledgments to a first-class "Credits" tab to provide clear and compliant attribution for all data sources (National Rail, TfL, bustimes.org).
- **Static Asset Integration**: Attributions will be pulled from the existing localized constants in the C++ firmware (where possible) or mirrored in the web portal's static structure.

**🐛 Active Quirks, Bugs & Discoveries**
- **Web Portal Size**: `web/index.html` is a monolithic ~4.3k line file. Any UI changes must be meticulously placed within the unified HTML/CSS/JS structure.
- **Existing Footers**: Credits already exist as small footers on all tabs; the new tab will consolidate and expand these.

**💻 Commands Reference**
- `pio run`: Full build (triggers `portalBuilder.py` to minify and gzip web assets).
- `node server.js`: Local web development server in `test/web`.

**🌿 Execution Environment**
- Device: ESP32 (not currently locked for this session).
- OS: MacOS.
- Testing: Local web server recommended for initial tab design.
