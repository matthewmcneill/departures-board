# ADR: Epoch Translation Matrix & Version-Locked Suffixing

## Status
Proposed/Implemented - 2026-04-11

## Context
The departures board was transitioning to a nested `feeds` schema (v2.6) for RSS and Weather configurations. Legacy configurations (Gadec-uk v1.0, v2.0) were flat. Storing a new configuration over the old `/config.json` was dangerous as it could lead to data loss or "breaking" the board's ability to revert to older firmware.

## Decision
1. **Decoupled Migration**: Logic was isolated into `GadecMigration` to prevent `ConfigManager` from becoming a "God Object" for legacy logic.
2. **Version Suffixing**: The board now saves valid configurations to versioned filenames (e.g., `/config_2_6.json`).
3. **Version Hunting**: `loadConfig` searches for the highest compatible versioned file before falling back to the legacy `/config.json`.
4. **Non-Destructive Baseline**: The original `/config.json` is preserved as a fallback/baseline.
5. **Storage Rotation**: A "Keep 3" pruning strategy ensures only the 3 most recent configuration versions occupy flash space.

## Consequences
- **Storage**: Increased storage usage in the short term (4 configuration files max), but bounded by pruning.
- **Safety**: Robust against failed migrations or firmware rollbacks.
- **Security**: Explicitly deletes `apikeys.json` after successful migration to the encrypted `.bin` format.
