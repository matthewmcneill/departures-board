# Architectural Evaluation: PlatformIO Dependency Management

This document critically evaluates two distinct strategies for managing the internal modules (`displayManager`, `weatherClient`, `webServer`, etc.) within the `departures-board` codebase.

---

## Approach A: The Monolithic Build (Current Configuration Strategy)
**Methodology:** Delete all `library.json` files. Rely entirely on the global `platformio.ini` file to define all `-I modules/...` include paths and `lib_deps` external dependencies.

### Strengths
> [!TIP]
> **Single Source of Truth**
> All project configuration exists in exactly one place. If you ever need to upgrade `ArduinoJson` or `U8g2`, you update one line in `platformio.ini`. There is zero risk of version drift or downgrades caused by a forgotten inner `library.json` file quietly requesting an older version.

- **Immunity to LDF Boundary Bugs**: PlatformIO's Library Dependency Finder builds a strict dependency graph. Without inner `library.json` files, PlatformIO simply treats the `modules/` folder as raw C++ source paths. It recursively compiles *everything* seamlessly, permanently eliminating the bizarre "vanished subdirectory" linker errors we experienced with the `fonts.cpp` file.
- **Predictable Build Pipeline**: Because you are manually defining the include paths via compiler flags (`-I`), the C++ preprocessor knows exactly where to look.

### Weaknesses
- **Zero Portability**: The modules are cemented into the `departures-board` repository. If you wanted to extract `weatherClient` and drop it into a completely new embedded project, it would break instantly because it doesn't carry a manifest explaining that it requires `ArduinoJson`.
- **Global Scope Pollution**: If one module needs a very specific, aggressive build flag (e.g., heavily optimized `O3` compilation), you are forced to define it globally in `platformio.ini`, which applies it to unrelated modules as well.

---

## Approach B: The Modular Component Architecture (LDF Driven)
**Methodology:** Keep strict `library.json` files at the root of `modules/*`. Remove the hardcoded `-I modules/...` build flags from `platformio.ini`. Allow PlatformIO's LDF (via `lib_extra_dirs`) to auto-discover the modules, read their JSON manifests, and resolve dependencies natively via `#include` tracing.

### Strengths
> [!TIP]
> **Strict Encapsulation**
> Every module acts as a self-contained, independent micro-library. `displayManager` explicitly declares, "I rely on `U8g2` and I export these headers."

- **Maximum Reusability**: You can drag and drop your custom modules into any future PlatformIO project effortlessly. The LDF will read the `library.json` and automatically fetch the external libraries the module needs.
- **Minimalist `platformio.ini`**: The global project configuration file becomes incredibly clean and terse, delegating configuration down to the logical components that actually own the domain.

### Weaknesses
> [!WARNING]
> **The Subdirectory Recursion Trap**
> This is exactly what broke the build earlier. When LDF recursively scans a module, encountering a nested `library.json` acts as a physical wall. It treats the subdirectory as a completely isolated component. Unless the parent explicitly declares a dependency on that subdirectory within its own JSON file, the entire directory is dropped from compilation. 

- **Dependency Conflicts**: If `displayManager/library.json` requests `ArduinoJson ^6.0.0` but `weatherClient/library.json` requests `ArduinoJson ^7.2.0`, the LDF might silently fail or lock the dependencies, leading to runtime crashes.
- **Slower Compilations**: You are currently running `lib_ldf_mode = deep+`. Asking the LDF to unspool the entire C++ preprocessor tree across 10 independent JSON-driven micro-libraries takes significantly more computational time before the compiler even starts.

---

## Final Assessment & Recommendation

The fundamental question is: **Are these modules bespoke to the departures board, or are they general-purpose open-source libraries meant to be shared?**

Because `departures-board` is a highly specialized embedded application with tightly coupled inter-module dependencies (e.g., `displayManager` relying on layouts containing board-specific logic), attempting to force strict encapsulation via Approach B introduces **high build fragility for minimal practical benefit.** 

**Recommendation: Fully commit to Approach A.** 
Delete the remaining `library.json` files. Embrace `platformio.ini` as the monolithic orchestrator. It guarantees structural stability, prevents hidden LDF linking omissions, and significantly lowers the cognitive load of maintaining external dependency versions.
