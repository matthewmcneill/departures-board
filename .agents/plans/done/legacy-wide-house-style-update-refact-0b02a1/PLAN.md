---
name: "wide House Style Update: Refactoring Attribution"
description: "Applied the new refactoring attribution line to the standard module header across the entire C++ codebase (87 files). Optimized the build system configuration to prioritize hardware targets and rename..."
created: "2026-03-16"
status: "DONE"
commits: ['011bd9f', '10a9bbf', 'b185905']
---

# Summary
Applied the new refactoring attribution line to the standard module header across the entire C++ codebase (87 files). Optimized the build system configuration to prioritize hardware targets and renamed the host-side test environment to `unit_testing_host`. Enhanced the agent skills infrastructure by adding targeted triggers to the `house-style-documentation` skill and migrating the project TODO list to the `.agents/` directory.

## Key Decisions
- **Automated Replacement**: Used `sed` with a repository-wide `find` search to ensure 100% consistency across all `.cpp` and `.hpp` files.
- **Documentation Alignment**: Synchronized the project's "House Style" definition and added explicit triggers (`review-ip`, `ip-review`) to the agent skills to ensure consistent policy enforcement.
- **Build Optimization**: Configured `default_envs` in `platformio.ini` and renamed the native environment to `unit_testing_host`. This provides semantic clarity and prevents hardware-dependency build errors when running standard compilation commands.
- **Infrastructure Consolidation**: Migrated legacy `TODO.md` to `.agents/todo_list.md` and removed obsolete `.github` rules to centralize all agent logic.

## Technical Context
