---
name: "Epoch Translation Matrix & Version-Locked Suffixing"
description: ""
created: "2026-04-06"
status: "DONE"
commits: ["eae03d0"]
---

# Summary

This plan successfully migrated the ESP32 Departures Board's configuration system from a legacy flat structure to the modern nested `feeds` schema (v2.6+). 

Key achievements:
- **Migration Matrix**: Decoupled epoch detection and translation logic into a dedicated `GadecMigration` utility, ensuring backward compatibility with V1.x and V2.x legacy formats.
- **Version-Locked Storage**: Transitioned from a single `/config.json` file to version-locked suffixing (e.g., `/config_2_6.json`). This protects the system from configuration corruption during firmware updates.
- **Pruning Policy**: Implemented a "Keep 3" rotation strategy to manage storage space, retaining the base `config.json` and the three most recent versioned files.
- **Secure Key Eradication**: Ensured that the plaintext `apikeys.json` is eradicated from disk after successful migration to the encrypted `apikeys.bin` blob.

## Technical Context
- [sessions.md](sessions.md)
- [context_bridge.md](context_bridge.md)
- [task.md](task.md)
