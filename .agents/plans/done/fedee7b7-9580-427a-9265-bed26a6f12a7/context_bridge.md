# Context Bridge

**📍 Current State & Focus**
The user investigated why the `/api/config` export contained two specifications for RSS feeds (`system.rssUrl` and `feeds.rss`). We discovered this was a legacy mapping in `webHandlerManager.cpp`. An implementation plan has been drafted and audited to permanently migrate the physical schema on the ESP32 to nest these under a `feeds` block (`configVersion 2.6`).

**🎯 Next Immediate Actions**
Execute the implementation plan by updating `ConfigManager` save/load logic to operate on the `feeds` nested block, adding the v2.6 schema migration boot override, and cleaning up the legacy `system` mappings in the API export.

**🧠 Decisions & Designs**
We are bumping `configVersion` to 2.6 to afford a transparent auto-upgrade of flash storage. When legacy devices boot, `ConfigManager` will detect version `< 2.6f`, nest the flat parameters, and issue an immediate secure `save()`.

**🐛 Active Quirks, Bugs & Discoveries**
The `webHandlerManager.cpp` currently translates between the two paradigms blindly. Removing `system.rssUrl` from the `/api/config` export means we must ensure the Vue.js frontend bindings solely rely on `feeds.rss`. Tests indicate they already do.

**💻 Commands Reference**
- Build/Test portal dependencies: `cd test/web && npm run test`

**🌿 Execution Environment**
- Native simulator running: `./tools/layoutsim/layoutsim.sh`
- Local web test server: `node test/web/server.js`

**⚠️ PORTABILITY RULE**
All target references in the plan (`modules/configManager/configManager.cpp`, `modules/webServer/webHandlerManager.cpp`) use relative paths.
