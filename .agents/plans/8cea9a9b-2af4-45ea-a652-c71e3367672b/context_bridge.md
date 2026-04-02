# Context Bridge

## 📍 Current State & Focus
We are undertaking a Dependency Injection and Encapsulation Architecture Refactor in the `departures-board` project. We have completed the research, traced the global usages, and formulated the implementation plan. We are currently paused right before execution begins.

## 🎯 Next Immediate Actions
Execute the steps in `implementation_plan.md` starting with eliminating the floating globals in `displayManager.hpp`, `weatherClient.hpp`, and `webServer.hpp`. Then relocate the callbacks from `systemManager` to use `appContext` directly.

## 🧠 Decisions & Designs
We decided that since `appContext` already owns the central singletons, the global external variables scattered throughout the codebase will be safely deleted. Callbacks originally shoved into `systemManager` for UI boot progress updates will be correctly rerouted as standard delegates passing through the `appContext`. `AsyncWebServer server` will be properly encapsulated into `WebServerManager`.

## 🐛 Active Quirks, Bugs & Discoveries
- We discovered that `extern DisplayManager displayManager;` hasn't been instantiated globally via ESP32 toolchains, meaning any usage of it fails if not using the simulator.
- `WebServerManager webServer` actually had a duplicate global definition shadowing the `appContext` member! We must clean this up efficiently.
- FreeRTOS yield handlers are present in some data sources; we must ensure these callbacks (`yieldCallbackWrapper`) remain intact when we decouple them from `systemManager`.

## 💻 Commands Reference
`pio run -e esp32dev` (to verify the build)

## 🌿 Execution Environment
PlatformIO (C++) and ESP32 hardware context.
