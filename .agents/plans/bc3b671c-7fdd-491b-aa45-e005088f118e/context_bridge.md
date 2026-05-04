---
title: Native Test Suite Stabilisation and ArduinoFake Hacks
distilled_at: 2026-04-26T22:33:00Z
original_plan_id: bc3b671c-7fdd-491b-aa45-e005088f118e
artifacts:
  - context_artifacts/adr_native_testing.md
  - context_artifacts/graveyard_native_testing.md
---

### Executive Summary
Fixed the `unit_testing_host` absolute linker errors and prevented an ArduinoFake `Unknown instance` crash on native `MockLittleFS` configuration writing. 

### Next Steps 
The test suite finally compiles successfully into a native macOS executable, but right now encounters an internal `SIGABRT` natively *immediately after* the `test_drawText_truncation` test run. 

You must isolate which framework call is triggering the abort trap (very likely an unmocked framework call being triggered by whatever sequence starts directly after drawing primitives) and mock the missing functionality or gracefully ignore it natively.

### Deep Context Menu
> [!WARNING]
> Do NOT read the detailed artifacts below unless your current task explicitly requires the deep context. If needed, use your read tool on `context_artifacts/[filename]`.

- `context_artifacts/adr_native_testing.md` - Context scoping for `appContext` and ArduinoFake Print overrides.
- `context_artifacts/graveyard_native_testing.md` - Failed attempts with LLDB batch backtraces and global variable locations in native tests.
