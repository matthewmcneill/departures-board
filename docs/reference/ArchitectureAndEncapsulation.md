# System Architecture: Encapsulation & Dependency Injection

This document outlines the architectural patterns used in the Departures Board project to ensure modularity, maintainability, and memory efficiency in an embedded C++ environment.

## 1. Central Orchestration: The AppContext Pattern

The `appContext` class serves as the single source of truth and lifecycle owner for all core system services. While the main entry point (`departuresBoard.cpp`) handles high-level hardware setup, it immediately delegates all logic to `appContext`.

### Why not use Globals?
Traditional Arduino/ESP32 projects often rely on global variables defined in the main file and accessed via `extern` in other modules. This project avoids this "spaghetti" pattern for several reasons:
- **Defined Initialization Order**: C++ does not guarantee the order of global variable initialization across different files. `appContext::begin()` provides a deterministic boot sequence.
- **Explicit Dependencies**: By passing a pointer to `appContext`, dependencies are made explicit rather than hidden behind global scope.
- **Unit Testability**: Subsystems can be tested in isolation by providing them with a mock or minimal implementation of the `appContext`.

## 2. Dependency Injection (DI)

We use a "Registry Selection" style of Dependency Injection. Instead of modules discovering each other through global state, they receive a reference to the `appContext` hub.

### Pattern: The `begin(appContext* ctx)` Method
Most manager classes follow this pattern:
```cpp
void DisplayManager::begin(appContext* context) {
    this->ctx = context;
    // Now DisplayManager can safely access:
    // ctx->getConfigManager();
    // ctx->getsystemManager();
}
```
This ensures that no module tries to access a service before the central orchestrator has validated its existence and readiness.

## 3. Encapsulation & Data Hiding

Following the **Single Responsibility Principle (SRP)**, each module is responsible for exactly one aspect of the system:
- `ConfigManager`: JSON persistence and settings.
- `DisplayManager`: u8g2 rendering and carousel logic.
- `SystemManager`: High-level state transitions and refresh timers.
- `NetworkManager`: WiFi connectivity and stack stability.

### The "Consumer" Pattern for Configuration
To avoid tight coupling between the `ConfigManager` and its users, we use a registration pattern:
1. Subsystems implement a notification interface (or simple method).
2. They register themselves with the `ConfigManager`.
3. When settings change (e.g., via the Web UI), `ConfigManager` notifies all registered consumers to re-apply their local state.

## 4. Embedded Memory Optimization

Despite using rich Object-Oriented patterns, the architecture is designed to be "zero-cost" or "memory-neutral" where possible:
- **Global Instance**: `appContext` is instantiated exactly once as a global object. This places it in DRAM (BSS/Data segment), the exact same footprint as if its members were separate globals.
- **Stack Safety**: Complex logic is encapsulated in methods rather than deep nested function calls with large local variables.
- **Heap Stability**: Long-lived managers are members of the global `appContext`, meaning they are allocated once at boot and never freed, eliminating heap fragmentation risks for the core system.

## Summary

The combination of a central `appContext` hub and explicit Dependency Injection transforms what would otherwise be a monolithic "God Object" in the main file into a collection of decoupled, testable, and maintainable services.
