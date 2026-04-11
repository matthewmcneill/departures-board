# ADR: RAII Scope Guard for Hybrid Memory Scrubbing

## Context
The `nrDARWINDataProvider` module uses a **Hybrid Memory Model**: utilizing `String` objects for flexible XML parsing (e.g., `tagName`, `tagPath`) while clearing them manually at the end of every fetch cycle to prevent long-term heap fragmentation.

Previously, this was handled via a legacy `goto scrub_and_exit_fetch` pattern. However, as the codebase modernized (adding `std::unique_ptr` and more complex object initializations), this pattern caused compilation errors because `goto` was jumping over the initialization of non-trivial objects.

## Decision
We have replaced the `goto` pattern with a **RAII Scope Guard**.

### Implementation Pattern
A local `ScrubGuard` struct is defined within the `executeFetch()` routine:

```cpp
struct ScrubGuard {
  nrDARWINDataProvider *p;
  ~ScrubGuard() {
    p->tagName.clear();
    p->tagPath.clear();
    p->grandParentTagName.clear();
    p->parentTagName.clear();
  }
} guard{this};
```

### Benefits
1.  **Safety**: Cleanup is deterministic and guaranteed regardless of the function's exit path (success or error).
2.  **Compatibility**: Resolves all "jump to label" compilation errors.
3.  **Modern Scoping**: Allows variables like `WiFiClientSecure` and `NationalRailStation` to be declared locally where they are first used, rather than being pre-declared at the top of the function.

## Status
**Accepted and Verified**. Successfully deployed to Nano ESP32 hardware.
