# 📍 Current State & Focus
Researching and planning a new feature to allow users to backup and restore their `/config.json` via the web portal. The current implementation plan has been drafted and is awaiting execution authorization.

# 🎯 Next Immediate Actions
1.  Implement new API routes in `modules/webServer/webHandlerManager.cpp`.
2.  Update `web/index.html` with the new "Maintenance" UI.
3.  Update the mock server in `test/web/server.js` for local testing.

# 🧠 Decisions & Designs
- **Scope**: Only `config.json` is backed up. Decrypted API keys and WiFi credentials are NOT included for security.
- **Backend Flow**: Use `AsyncWebServer` to serve the file and handle POST multipart or raw body for restore.
- **Frontend Flow**: Use standard `fetch` with `blob()` for downloads and `FileReader` with `FormData` for uploads.

# 🐛 Active Quirks, Bugs & Discoveries
- `DeviceCrypto` is used for keys and WiFi, making them hardware-bound. Re-restoring them to different hardware would fail unless keys are decrypted first (which is avoided).
- `AsyncWebServer` memory constraints: Serving large JSON (like the full config) could spike heap usage. A serialization guard is already in place in `WebHandlerManager`.

# 💻 Commands Reference
- `pio run`: Full build and minification check.
- `cd test/web && node server.js`: Local mock server.
- `/flash-test`: Final hardware deployment.

# 🌿 Execution Environment
- **Branch**: main
- **Hardware**: Nano ESP32 (simulated and physical tests required).
