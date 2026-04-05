# Tasks
- `[x]` Implement 15-Second Post-Departure Refresh (RDM)
    - `[x]` Include AppContext in `nrRDMDataProvider.cpp`
    - `[x]` Calculate Wall Clock Delta inside `executeFetch()`
    - `[x]` Compute Custom Interval clamped between 15s and 45s
    - `[x]` Apply calculated interval via `setNextFetchTime`
- `[x]` Verification
    - `[x]` Compile code natively
    - `[x]` Verify system logs
