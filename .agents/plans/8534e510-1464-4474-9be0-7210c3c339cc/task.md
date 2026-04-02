# Refactoring Display Board allocation architecture

- [ ] Draft an implementation plan and request user review
- [ ] Refactor `displayManager.hpp` to decouple concrete board implementations
- [ ] Create `boardFactory.hpp` and `boardFactory.cpp` to instantiate concrete boards dynamically
- [ ] Refactor System Board lifecycle and allocation
- [ ] Update `departuresBoard.cpp` and bootstrap code to use the new architecture
- [ ] Verify there is zero heap fragmentation during steady-state normal operation
