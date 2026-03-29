# Context Bridge

**📍 Current State & Focus**
The project is currently assessing a firmware migration and configuration upgrade testing strategy for the ESP32 Departures Board project. A legacy firmware testing approach was evaluated, and an improved "Synthetic Data Injection" approach using PlatformIO's LittleFS filesystem upload tools has been proposed in `migration_testing_strategy.md`.

**🎯 Next Immediate Actions**
1. Review the proposed `migration_testing_strategy.md`.
2. Proceed with establishing legacy fixtures in a `test/fixtures/` directory (e.g., `< v2.0` `apikeys.json` and `< v2.3` `config.json`).
3. Create a lightweight setup script to orchestrate the `pio run -t uploadfs` test suite.

**🧠 Decisions & Designs**
- Bypassing manual hardware flashing and UI-based configuration input.
- Creating static legacy JSON data mocks based on older schematic payloads instead of producing them from physical ESP32 extractions.

**🐛 Active Quirks, Bugs & Discoveries**
- No major bugs discovered. 
- Discovered that PlatformIO `uploadfs` is highly compatible with the current CI/CD strategy and circumvents compiling legacy C++ codebase.

**💻 Commands Reference**
- `pio run -t uploadfs` (upload legacy configurations securely)
- `pio run -t upload` (deploy new firmware overlaying legacy config)
- `pio device monitor` (for validation)

**🌿 Execution Environment**
- Assessing migration tests on standard ESP32 target hardware architecture.
- Waiting for user approval on architectural approach before proceeding to execution.
