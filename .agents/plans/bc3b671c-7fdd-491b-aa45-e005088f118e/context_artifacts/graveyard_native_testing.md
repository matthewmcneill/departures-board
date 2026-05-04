# Graveyard: Native Compilation 

1. **Defining Global Objects Earl**: Defining `appContext` inside `test/mocks/Stubs.cpp` or high up in `test_main.cpp` completely broke header inclusions because `appContext` as a variable masked standard object type lookups inside layout module constructors.
2. **LLDB Batch Issues:** Attempted LLDB automated execution `lldb --batch -o run -o bt -o quit` to catch `SIGABRT` output from `ArduinoFake()`. This fails with timeouts in the background and struggles to reliably dump stack traces back to MCP tools cleanly. The better approach moving forward is to simply selectively comment out the `test_main.cpp` runner blocks until the exact crashing suite is identified.
