# Refactoring Display Board allocation architecture

- [x] Draft an implementation plan and request user review
- [x] Refactor `displayManager.hpp` to decouple concrete board implementations
- [x] Create `boardFactory.hpp` and `boardFactory.cpp` to instantiate concrete boards dynamically
- [x] Refactor System Board lifecycle and allocation
- [x] Update `departuresBoard.cpp` and bootstrap code to use the new architecture
- [x] Verify there is zero heap fragmentation during steady-state normal operation
Refactored `displayManager.hpp` to decouple concrete board implementations.
Created `boardFactory.hpp` and `boardFactory.cpp` to instantiate concrete boards dynamically.
Refactored System Board lifecycle and allocation.
