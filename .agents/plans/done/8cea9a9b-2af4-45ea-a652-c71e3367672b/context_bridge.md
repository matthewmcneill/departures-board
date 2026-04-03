# Context Bridge

## 📍 Current State & Focus
Completing the Dependency Injection and Encapsulation Refactor (Plan `8cea9a9b-2af4-45ea-a652-c71e3367672b`). The project is in a stable, running state, having successfully eliminated floating global singletons and resolved UI-bleeding callbacks. We are packaging the finalized context into the artifact directory.

## 🎯 Next Immediate Actions
- Proceed with any pending items on the `.agents/todo_list.md` or instantiate new `/plan-spawn` instances for broader organizational goals.

## 🧠 Decisions & Designs
- Architecture strictly enforces a Dependency Injection pattern where the `appContext` acts as the service hub.
- Global singletons (like `displayManager` and `currentWeather`) were fully removed from global namespaces.
- `AsyncWebServer` logic is encapsulated completely within a heap-allocated pointer inside `WebServerManager`.

## 🐛 Active Quirks, Bugs & Discoveries
- Header inclusion ordering and incomplete type definitions must be carefully managed. When adding new modules, ensure `.cpp` files include `<Arduino.h>`, and forward declarations are used in `.hpp` interfaces.

## 💻 Commands Reference
- Build test: `pio run -e esp32dev`
- Unit tests: `pio test -e unit_testing_host`

## 🌿 Execution Environment
- **Branch**: main
- **Environment**: esp32dev
