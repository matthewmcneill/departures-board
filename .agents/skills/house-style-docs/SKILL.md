---
name: house-style-documentation
description: Consistently and thoroughly document C++ code following the project's "house style". Use this skill whenever documenting, refactoring, or creating new C++ modules to ensure they meet the specific header, method, and inline comment standards.
---

# House Style Documentation

Enforce a specific set of documentation standards for C++ header and source files.

## High-Level Standards

1.  **Module Headers**: Every file must have a standard header block.
2.  **Function/Method Documentation**: All public and private functions/methods must have Doxygen-style comments.
3.  **Variable/Constant Documentation**: Module-level scoped items must have same-line comments.
4.  **Functional Flow**: Longer functions must have block comments describing the "why" and "how" of the process.
5.  **Arcane Logic**: Complex or non-obvious code lines must be explained.
6.  **Naming Conventions**: File names and library names must use `camelCase`.
7.  **Implementation Plans**: All implementation plans must be reviewed for house style (Goal, Review Required, Proposed Changes, Verification).

---

## 1. Module Headers

Every file MUST start with the standard license header, followed by `Module:`, `Description:`, and `Exported Functions/Classes:`.

### Format:
```cpp
/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: [file path relative to repo root]
 * Description: [multi-line description of the module's responsibility]
 *
 * Exported Functions/Classes:
 * - [Name]: [Single-line description]
 * - [ClassName]: [Class description]
 *   - [MethodName](): [Single-line description]
 *   - [VariableName]: [Single-line description]
 */
```

> [!IMPORTANT]
> The "Exported Functions/Classes" list is critical. It should allow a developer to quickly see the services offered by the module without reading the whole file. Provide a one-line summary for EVERY externalized method, property, and variable.

---

## 2. Function and Method Documentation

Use Doxygen-style comment blocks for all declarations and implementations.

### Format:
```cpp
/**
 * @brief [Concise summary of what the function does]
 * @param [paramName] [Description of the parameter]
 * @return [Description of the return value, if applicable]
 */
void myFunction(int param1);
```

---

## 3. Scoped Variables and Constants

For variables and constants defined at the module level (headers or file-scope in `.cpp`), add a short description on the same line.

### Example:
```cpp
#define MAX_RETRY_COUNT 5 // Maximum number of connection attempts before failing
int currentRetryCount = 0; // Tracking variable for active connection attempts
```

---

## 4. Internal Functional Flow

For functions longer than ~15 lines, use block comments to group logical steps and explain the flow.

### Example:
```cpp
void complexProcess() {
  // --- Step 1: Initialize hardware ---
  // Ensure the peripheral is in a clean state before signaling
  ...

  // --- Step 2: Protocol Handshake ---
  // Negotiate speed and duplex with the remote peer
  ...
}
```

---

## 5. Arcane and Complex Logic

Identify lines of code that are not self-explanatory (bit manipulation, complex pointer arithmetic, hardware-specific quirks) and provide an explanation.

### Example:
```cpp
uint8_t flags = (data >> 4) & 0x0F; // Extract 4-bit status nibble from high byte
```

## 6. Naming Conventions

All file names and library names MUST follow the `camelCase` naming convention. This ensures consistency across the codebase.

### Requirements:
- **File Names**: Use `camelCase.cpp` or `camelCase.hpp` (e.g., `displayManager.cpp`, `wifiConfig.hpp`).
- **Library Names**: External libraries or internal modules should be referred to using `camelCase` in documentation and configuration.

---

## 7. Implementation Plans

Whenever an implementation plan is produced, it MUST be reviewed and updated to adhere to the project's house style.

### Structure:
- **Goal Description**: Clear, concise explanation of the objective.
- **User Review Required**: Highlight critical decisions or breaking changes using GitHub alerts.
- **Proposed Changes**: Grouped by component, using `[MODIFY]`, `[NEW]`, and `[DELETE]` tags with absolute file links.
- **Verification Plan**: Practical steps for automated and manual verification.

---

## Workflow

1.  Read the target file OR implementation plan.
2.  Identity missing or substandard documentation/content based on the rules above.
3.  Ensure the file name and module naming follow the `camelCase` requirement.
4.  For implementation plans, ensure all standard sections are present and correctly formatted.
5.  Regenerate the content with the improved house style.
6.  Ensure existing logic or plan details are PRESERVED exactly; only formatting and clarity should change.
