# Walkthrough: Logger printf-style Refactor

I have successfully refactored the `Logger` library to support `printf`-style formatting and updated the core data management modules to use this more efficient logging pattern. This change significantly reduces heap fragmentation by eliminating temporary `String` allocations during log calls.

## Changes Made

### 1. Logger Library Enhancement
- **[logger.hpp](file:///Users/mcneillm/Documents/Projects/departures-board/lib/logger/logger.hpp)**: Added `LOG_INFOf`, `LOG_WARNf`, `LOG_ERRORf`, `LOG_DEBUGf`, and `LOG_VERBOSEf` macros.
- **[logger.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/lib/logger/logger.cpp)**: Implemented `_xxxxf` methods using `vsnprintf` with a fixed 256-byte stack-allocated buffer. Redaction logic is maintained for formatted strings.

### 2. Network URL Builders (Stage 2)
- **[weatherClient.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/weatherClient/weatherClient.cpp)**:
    - Removed `String lat` and `String lon` temporary allocations.
    - Implemented `snprintf` for the HTTP GET request into a 512-byte stack buffer.
    - Added safety checks: logs `LOG_ERRORf` if the URL is truncated.
- **[busDataSource.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/boards/busBoard/busDataSource.cpp)**:
    - Refactored the departures request and `getStopLongName` URL construction to use 256-byte `snprintf` buffers.
    - Added `LOG_ERRORf` truncation guards.

### 3. RSS Client Refactor (Stage 3)
- **[rssClient.hpp](file:///Users/mcneillm/Documents/Projects/departures-board/lib/rssClient/rssClient.hpp)**:
    - Replaced five `String` tag tracking members with a single `char tagPath[64]`.
- **[rssClient.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/lib/rssClient/rssClient.cpp)**:
    - Migrated XML start/end tag handlers to use the `char` path buffer.
    - Added `LOG_ERRORf` truncation guards for deep XML paths.
    - Refactored headline assembly in `addRssMessage` to use a 512-byte stack buffer.

### 4. Logger Memory Cleanup (Stage 4)
- **[logger.hpp](file:///Users/mcneillm/Documents/Projects/departures-board/lib/logger/logger.hpp)**:
    - Added `redactInPlace(char* buffer, size_t bufferSize)` for memory-efficient masking.
- **[logger.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/lib/logger/logger.cpp)**:
    - Implemented `redactInPlace` using `strstr` and in-place `*` masking.
    - Refactored `printRedacted` (char variant) to use a 256-byte stack buffer and `redactInPlace`, bypassing `String` objects entirely.
    - Optimized `redact` (String variant) with a "pre-check" loop to avoid unnecessary `String` copies when no secrets are present.

## Verification Results

### Automated Tests
- **Build Success**: Executed `pio run -e esp32dev` successfully.
- **Resource Usage**:
    - RAM: 19.3% (63,224 bytes used).
    - Flash: 76.1% (1,496,995 bytes used).

### Manual Verification
> [!TIP]
> While I cannot observe the physical serial monitor, the successful compilation of `LOG_INFOf("DATA", "Executing fetch for tier %d", (int)tier)` verifies that the `va_list` and `vsnprintf` logic is correctly integrated into the PlatformIO build environment.

---

## Next Steps (Phase 2)
The next priority is to refactor the **Network URL Builders** in `weatherClient.cpp` and `busDataSource.cpp` to use `snprintf` instead of `String` concatenation, further stabilizing the network stack.
