# Memory Stability Refactoring (String Removal)

The goal of this refactoring is to remove all usages of the dynamically allocating `String` object from long-running network parsers in the firmware. Because `String` resizes dynamically, using it heavily in continuous operations like JSON stream parsing causes severe heap fragmentation on the ESP32. We will replace `String` variables with C-style bounded `char` arrays and `const char*` pointers to ensure 24/7 memory stability.

> [!IMPORTANT]  
> I have explicitly ingested and reviewed the following project rules prior to drafting this plan:
> - `.agents/rules/explicit-execution.md`
> - `.agents/rules/implementation-plan-review.md`
> - `.agents/rules/queue-enforcement.md`
> - `.agents/rules/web-testing-strategy.md`

[x] Reviewed by embedded-systems - Passed (Resource constraints correctly identified: Flash memory increases marginally due to fixed allocations, but heap fragmentation risk drops to near-zero).  
[x] Reviewed by house-style-docs - Passed (Naming conventions and structures adhere to house style).  
[x] Reviewed by architectural-refactoring - Passed (Encapsulation and dependency management preserved).  

## User Review Required

> [!WARNING]  
> To guarantee maximum safety against unexpected server payloads without unnecessarily wasting RAM, I have set the bounds for the JSON array state buffers to **64 characters** for both `currentKey` and `currentObject`. Based on research into the `weatherStatus` configuration (`description[46]`) and the `busDataSource` lengths (`BUS_MAX_LOCATION = 45`), 64 bytes is incredibly safe.

## Proposed Changes

---

### Display Manager Module (busDataSource)

We will remove all `String` usage from the parser state and utilize bounded zero-initialized buffers.

#### [MODIFY] [busDataSource.hpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/boards/busBoard/busDataSource.hpp)
- Change `String currentKey = ""` to `char currentKey[64] = ""`.
- Change `String currentObject = ""` to `char currentObject[64] = ""`.
- Change `String longName` to `char longName[80]`.
- Change `void key(String key)` to `void key(String key)`
  - *Note*: `JsonListener.h` mandates `void key(String key) override;`, we cannot change the signature. However, we can avoid string concatenation within our own state.
- Change `String stripTag(String html)` to `void stripTag(char* inputBuffer)` (in-place modification) to eliminate hidden copies.

#### [MODIFY] [busDataSource.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/boards/busBoard/busDataSource.cpp)
- Update parser tracking to use `strncpy(currentKey, key.c_str(), sizeof(currentKey)-1)` instead of assignment.
- Remove dynamic string concatenation in `busDataSource::executeFetch` (e.g. constructing `request` header dynamically). Move to `snprintf`.
- Update the HTML scraper to avoid instantiating temporary `String line` copies where possible, or refactor `line.indexOf()` parsing to zero-copy `strstr()`. 

---

### Weather Client Module (weatherClient)

Similar to the Bus module, we will eliminate `String` cache storage and concatenations, improving long-term HTTP background thread stability.

#### [MODIFY] [weatherClient.hpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/weatherClient/weatherClient.hpp)
- Change `String currentKey = ""` to `char currentKey[64] = ""`.
- Change `String currentObject = ""` to `char currentObject[64] = ""`.
- Change `String activeApiKey = ""` to `char activeApiKey[48] = ""`.

#### [MODIFY] [weatherClient.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/weatherClient/weatherClient.cpp)
- Rewrite `executeFetch()` networking header generation to use `snprintf` instead of `String` + `String` concatenation.
- Update `key()` and `value()` overrides to compare strings directly off the C-string pointer `key.c_str()` using `strcmp` rather than allocating `String` equality checks (e.g., replace `currentKey == F("temp")` with `strcmp(currentKey, "temp") == 0`).

---

### System Manager Module

#### [MODIFY] [systemManager.hpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/systemManager/systemManager.hpp)
- Change `String getBuildTime();` to `const char* getBuildTime();`.

#### [MODIFY] [systemManager.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/systemManager/systemManager.cpp)
- Update `getBuildTime()` to return a `static char` buffer or `const char*` macro string instead of allocating a new `String` on the heap every call.

## Verification Plan

### Automated Tests
- Run `pio test -e unit_testing_host` to ensure logic remains intact if tests cover parsing.

### Manual Verification
- Compile and execute the native `.agents/workflows/web-testing-strategy.md` Phase 2 checks by running `pio run -e esp32dev` to verify there are absolutely no linker or compiler errors.
- Confirm memory watermarks look healthy by visually verifying `LOG_DEBUG` stack outputs before/after.
