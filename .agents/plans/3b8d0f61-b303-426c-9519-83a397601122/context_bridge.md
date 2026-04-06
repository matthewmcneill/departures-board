# Context Bridge

## 📍 Current State & Focus
We have completed a comprehensive redesign of the OTA architecture, focusing on a robust "zero-touch" appliance model that prioritizes usability over strict identity verification, but retains payload validation via software-level cryptographic signing using `mbedtls` and an AES-256 encrypted private key. The implementation plan detailing the `gh` release script generation, `otaQuietHour` caching, and API/rollback mechanisms is complete and verified by the user. 
We have documented the OTA system mechanics in `docs/reference/OtaArchitecture.md` and linked it to the `SystemSpecificationDocument.md`. The design phase is finished, and the plan is queued for implementation.

## 🎯 Next Immediate Actions
1. Follow the generated `implementation_plan.md` to create `scripts/generate_keys.sh` and `scripts/deploy_release.py`.
2. Refactor `lib/HTTPUpdateGitHub/HTTPUpdateGitHub.cpp` to integrate signature downloading and `mbedtls_sha256` hashing verification prior to bootloader partition flips.
3. Update `modules/configManager/` components and `lib/otaUpdater/otaUpdater.cpp` to encapsulate polling inside `otaQuietHour` checks.
4. Abstract checking and forced updating endpoints into `modules/webServer/webHandlerManager.cpp`.
5. Implement the rollback feature hook.

## 🧠 Decisions & Designs
- **Insecure Transport / Signed Payload**: Bypassing GitHub CA verification (`setInsecure()`) ensures the appliance doesn't permanently brick when the Root CA expires. Tampering vectors are mitigated by forcing the firmware to hash the downloaded payload against a downloaded `.sig` signature using `mbedtls` and an embedded public key.
- **Rollback Hook**: We are exposing native ESP32 partition flipping via a manual Web Portal `/api/ota/rollback` endpoint to instantly recover from severe bugs.
- **OTA Quiet Hour**: Daily `tick()` checks will be strictly sequestered to `3:00 AM` by default to avoid screen takeover.

## 🐛 Active Quirks, Bugs & Discoveries
- N/A

## 💻 Commands Reference
- Normal compilation flow: `pio run -e esp32dev`

## 🌿 Execution Environment
- Target Environment: ESP32 dual core (`esp32dev`)
- OS: General local testing (firmware deployment handled externally).
