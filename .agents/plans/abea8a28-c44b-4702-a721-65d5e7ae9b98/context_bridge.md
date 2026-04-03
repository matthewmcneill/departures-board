# Context Bridge

- **📍 Current State & Focus**: Completely finished drafting and reviewing the "NVS Security Migration" architectural plan. Iterated the design based on user feedback to use a `DeviceCrypto` library class injected via `appContext` rather than polluting `ConfigManager`. The plan is formally queued and ready for execution.
- **🎯 Next Immediate Actions**: Claim the hardware lock via `/plan-start`. Create the `lib/deviceCrypto` abstraction, bind it to `appContext`, and migrate `wifiManager.cpp` and `configManager.cpp` as detailed in `implementation_plan.md`.
- **🧠 Decisions & Designs**: 
  - **Hybrid Security**: AES-256-CBC encryption via `mbedtls` using a hardware-bound key stored in NVS, protecting JSON payloads residing on LittleFS. This prevents partition-shift OTA bricking and out-of-memory errors.
  - **Single Responsibility**: The security wrapper will be a discrete library module (`lib/deviceCrypto`), keeping dependency injection pure and avoiding SRP violations inside `ConfigManager`.
- **🐛 Active Quirks, Bugs & Discoveries**: NVS partition is rigidly locked to an un-expandable 20KB space inside current `csv` definitions without destroying OTA boundaries, rendering straight token-migrations risky due to page exhaustion out-of-memory errors (`ESP_ERR_NVS_NO_FREE_PAGES`). 
- **💻 Commands Reference**: 
  - Compile constraints: `pio run -e esp32dev -e unit_testing_host`
- **🌿 Execution Environment**: Active git branch. Hardware is attached (`safe-flash.sh` is monitored).
- **⚠️ PORTABILITY RULE**: You MUST use repository-relative paths (e.g., `modules/foo/bar.cpp`) for all in-repo file references. You are **STRICTLY PROHIBITED** from using absolute paths starting with `/Users/` (or equivalents) for project files.
