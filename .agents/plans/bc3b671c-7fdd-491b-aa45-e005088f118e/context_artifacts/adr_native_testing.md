# ADR: Native Compilation Scope collisions & Native File System Writes

Decisions taken during `unit_testing_host` debugging to achieve a complete build without linker or early crash issues:

1. **`appContext` name collision:** We moved the definitions of the `appContext` globals (e.g., `class appContext appContext{};`) to the absolute EOF of `test_main.cpp`. This avoids the type collision that causes `must use 'class' tag` errors inside `#include` layout classes.
2. **Bypassing ArduinoFake for file writing:** When natively saving to `<FS.h>` via our local `MockLittleFS.cpp` bridge, calling `.print()` inside `ConfigManager::saveFile` caused `ArduinoFake` to intercept `Print::print` and immediately throw an `Unknown instance` runtime error. Going forward, when natively handling `fs::File`, we bypass ArduinoFake completely by dispatching directly to the lower level `.write((uint8_t*)buf, len)`, which successfully writes the real payload without hitting the `Print` fake inheritance tree.
