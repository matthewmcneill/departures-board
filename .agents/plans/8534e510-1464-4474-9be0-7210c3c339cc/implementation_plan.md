# DisplayManager Allocation Architecture Refactor

[x] Reviewed by `house-style-documentation` - passed
[x] Reviewed by `architectural-refactoring` - passed (SRP, OCP, and DIP observed)
[x] Reviewed by `embedded-systems` - passed (Heap impact explicitly analyzed, Resource Impact Assessment added)

Architectural refactor of the DisplayManager to use a Factory Pattern and dynamic polymorphism, removing `std::variant` and hardcoded concrete configurations to conform to the Open/Closed Principle.

## User Review Required
> [!WARNING]
> Moving from statically allocated member components for System Boards (e.g. `splashBoard`, `wizardBoard`) to dynamic heap allocation allows completely decoupling `displayManager.hpp` from the concrete headers. 
> 
> My proposal is to move all system boards to pointers (`iDisplayBoard* systemBoards[MAX_SYSTEM_BOARDS]`) and allocate them at boot in `setup()` or `applyConfig()`. Since this occurs once, fragmentation is a non-issue. Please review this approach; alternatively, we can maintain them as separate `iDisplayBoard* splashBoard` etc members inside `DisplayManager`.

## Proposed Changes

### displayManager
Summary: Removal of `std::variant`, concrete board headers, and conversion to polymorphic arrays.

#### [MODIFY] [displayManager.hpp](file:///Users/mattmcneill/Personal/Projects/departures-board/modules/displayManager/displayManager.hpp)
- Remove all `#include <boards/...>` concrete implementations.
- Replace `BoardVariant slots[MAX_BOARDS];` with `iDisplayBoard* slots[MAX_BOARDS];` initialization to `nullptr`.
- Replace explicit system board members (`SplashBoard splashBoard`, etc.) with pointer members (e.g., `iDisplayBoard* splashBoard`) or an array map mapped to `SystemBoardId`.
- Add `#include "boardFactory.hpp"` dependency.

#### [MODIFY] [displayManager.cpp](file:///Users/mattmcneill/Personal/Projects/departures-board/modules/displayManager/displayManager.cpp)
- Update code to use pointers for display logic (e.g. `slots[i]->tick()`).
- In `clearSlot`, implement proper cleanup (`delete slots[i]`) making sure we check for `nullptr`. 
- Ensure `applyConfig()` leverages the `BoardFactory` to build new boards and delete old ones sequentially.

### BoardFactory
Summary: Introduction of the Factory pattern module to handle dependency instantiation.

#### [NEW] [boardFactory.hpp](file:///Users/mattmcneill/Personal/Projects/departures-board/modules/displayManager/boardFactory.hpp)
- Define `BoardFactory` with global/static instantiation methods for `BoardType` and `SystemBoardId`.

#### [NEW] [boardFactory.cpp](file:///Users/mattmcneill/Personal/Projects/departures-board/modules/displayManager/boardFactory.cpp)
- Centralize all `#include <boards/...>` to this isolated file, isolating `#include` cascades.
- Implement the factory logic using `switch` statements to return new instances on the heap.

### Bootstrapping
Summary: Ensuring memory allocations are stable at startup.

#### [MODIFY] [departuresBoard.cpp](file:///Users/mattmcneill/Personal/Projects/departures-board/src/departuresBoard.cpp)
- Make sure that display startup initializes the system board instances via `BoardFactory` dynamically before proceeding to loop.

## Resource Impact Assessment

As required by the `embedded-systems` skill:
- **Memory (Flash/ROM)**: Minimal impact. Factory pattern uses slight overhead for virtual dispatch tables.
- **Memory (RAM/Heap)**: Peak heap usage remains relatively unchanged or slightly reduced. Dynamic allocation of active Boards means unused Boards (like Tube if only National Rail is selected) are completely unallocated, reducing runtime heap. Since changes only occur at configuration points, dynamic allocation will NOT cause steady-state heap fragmentation during normal operation. 
- **Memory (Stack)**: Local variables within factory remain tightly scoped. Negligible impact.
- **Power**: Power consumption remains unchanged.
- **Security**: No material change to security posture.

## Verification Plan

### Automated Tests
- Build verification via `platformio run`.

### Manual Verification
- View serial output via `/monitor` or run serial monitor tests (`/flash-test`).
- Use the serial heartbeat logs `Alive | Heap: [X] (Max Block: [Y])` to track memory in steady state. Over a 5-minute uptime, verify that `Max Block` and `Heap` are stable, indicating zero runtime fragmentation.
- Verify transitions between different user boards and system boards work flawlessly.
