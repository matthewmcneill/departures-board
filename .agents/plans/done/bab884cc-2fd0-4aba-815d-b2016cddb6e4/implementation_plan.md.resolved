# Arduino String Optimization (Phase 1)

Refactor high-frequency `String` usage to improve heap stability and reduce fragmentation risks, focusing primarily on the `Logger` system and API clients.

## User Review Required
> [!IMPORTANT]
> The `Logger` refactor will change the `LOG_*` macros to support `printf`-style formatting. This is a breaking change for any custom code that expects `String` concatenation as the only way to log.

## Proposed Changes

### Logger Optimization (Phase 1)
Summary: Update `Logger` to handle `const char*` directly and provide a `printf`-style interface.

#### [MODIFY] [logger.hpp](lib/logger/logger.hpp)
- Add `LOG_INFOf`, `LOG_WAROf`, `LOG_ERRORf`, `LOG_DEBUGf` macros.
- Add `printf`-style methods to the `Logger` class using `va_list`.

#### [MODIFY] [logger.cpp](lib/logger/logger.cpp)
- Implement `printf`-style methods that use `vsnprintf` into a fixed internal buffer (e.g., 256 bytes) to avoid heap allocations.

### API Client Optimization (Phase 2)
Summary: Replace URL concatenation with `snprintf`.

#### [MODIFY] [weatherClient.cpp](modules/weatherClient/weatherClient.cpp)
- Replace `String request = "GET..." + ...` with `char buffer[512]; snprintf(...)`.

#### [MODIFY] [busDataSource.cpp](modules/displayManager/boards/busBoard/busDataSource.cpp)
- Update `String request` concatenation to use `snprintf`.

## Open Questions
- What is the maximum acceptable log message length for our fixed buffer? (Proposed: 256 bytes).

## Verification Plan
### Automated Tests
- `pio run` to verify no compilation errors.
- Run `Logger` unit tests (if any exists) to verify formatted output.

### Manual Verification
- Observe serial monitor to verify logs are still correctly formatted and redacted.
- Check `Alive | Heap: [X]` over 10 minutes to verify reduced fragmentation during network fetches.
