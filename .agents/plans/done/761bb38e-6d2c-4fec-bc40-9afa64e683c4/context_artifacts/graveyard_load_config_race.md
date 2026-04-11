# Graveyard: Configuration Loading Issues

## Scope Error: `loadedVersion`
During the refactor of `loadConfig`, the variable `loadedVersion` was incorrectly declared inside a JSON extraction block, but then used outside at the end of the function for the version-hunting logic. This caused a compilation error.
**Fix**: Moved the declaration to the top of the function or ensured the hunting logic happens within the scope of successful file-open.

## Iterator Conflict: `Dir` vs `File` (LittleFS)
The original pruning logic attempt used `Dir dir = LittleFS.openDir("/")`. On ESP32 with the standard `LittleFS` library, this is not the preferred method; rather, it uses `File root = LittleFS.open("/"); File file = root.openNextFile();`.
**Fix**: Refactored the rotation loop to use `openNextFile()` on the root directory.

## Destructive Logic Trap
A naive implementation of "save to modern version" could accidentally delete the baseline `config.json` before verifying the new version is valid.
**Fix**: Always verify the new versioned file is successfully written and readable before considering the migration "complete" (though we currently preserve `config.json` indefinitely as a baseline).
