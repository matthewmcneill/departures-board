# Task: Stabilizing Web Portal (Mobile Crash Fix)

- `[ ]` Update Hardware Lock Reason
- `[/]` Phase 1: Instrumentation & Identification (Diagnostic)
  - `[ ]` Implement memory-safe logging in `WebHandlerManager.cpp`
  - `[ ]` Flash & Monitor heap usage during portal load
- `[ ]` Phase 2: Payload Optimization (The "Diet")
  - `[ ]` Create and populate `<symbol>` library in `index.html`
  - `[ ]` Replace duplicated SVGs with `<use>` tags
  - `[ ]` Update JS `app` object to use symbols
- `[ ]` Phase 3: Backend Mitigation (Serialization)
  - `[ ]` Implement Web Request Semaphore/Atomic Counter in `WebHandlerManager.cpp`
  - `[ ]` Add comprehensive documentation for the semaphore
- `[ ]` Phase 4: Verification
  - `[ ]` Run stress tests (Playwright mobile emulation)
  - `[ ]` Monitor serial logs for heap stability
