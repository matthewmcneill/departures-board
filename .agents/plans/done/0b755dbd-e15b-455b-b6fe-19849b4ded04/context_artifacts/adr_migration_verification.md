# Architectural Decision: Config Migration Verification

## Context
After updating `gadecMigration` to migrate legacy config files to the v2.6 nested feeds structure, initial tests suggested migration was failing because the board wasn't loading any feed logic. Further analysis showed it was just repeatedly creating a blank `config_2_6.json`.

## Finding
To verify the state, we baked MCP diagnostic tools (`get_file_raw` and `list_files`) into the physical `mcpServer.cpp` module, effectively allowing a live, on-device read of the LittleFS filesystem.

The audit revealed there were *no* configurations at all on the tested board (no `settings.json`, no `config_2_5*`), so the `ConfigManager` was correctly assuming a virgin state and generating a new blank dictionary.

## Decision
We removed the legacy fallback logic involving `.lastknowngood` from the `saveFile` method inside `ConfigManager`, since versioned history provides adequate recovery paths without complex manual file duplication. We verified that the internal runtime arrays populate correctly when serialized.
