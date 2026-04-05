---
description: Concludes the plan lifecycle including house style checks, validation, git commits, final plan save, and releasing locks.
---

1. Determine the active Plan ID based on your session lineage (e.g., by checking if this session's ID exists inside any `.agents/plans/*/sessions.md` file).
2. **Refactoring & Documentation**: Run the `house-style-documentation` and `architectural-refactoring` skills to ensure all modified code and `/docs` files are strictly aligned with project standards.
3. **Validation & Assessment**: Run `pio run` and `pio test` to ensure the codebase compiles cleanly and passes all web and C++ tests. (Note: if a full build was compiled successfully very recently, such as during a flash-test or test build, and no code has changed since then, you may skip the `pio run` build step). Update `.agents/todo_list.md` with any discovered technical debt or necessary test suite additions.
4. **Git Commit & Project Log**: Prepare a "rich" Git commit summarizing the architectural decisions and breaking changes. Log the session summary, commit ID, and a repository-relative link to the archived plan context: `[.agents/plans/done/[ID]/](.agents/plans/done/[ID]/)` into `.agents/project_log.md`.
5. Run an implicit `/plan-save` to ensure the final 100% completed `task.md` and `implementation_plan.md` and final clean states are safely persisted to `.agents/plans/[ID]/`.
6. Update the `.agents/plans/[ID]/PLAN.md` YAML frontmatter to set `status: DONE`.
7. Finalize the operation by physically moving the completed plan's directory into the archive using absolute paths: `mv .agents/plans/[Plan ID] .agents/plans/done/[Plan ID]`
8. Check `.agents/plans/lock.md`. If your session currently holds the Hardware Lock in `Locked By`, unconditionally release it by setting it to `NONE` and `Reason` to `NONE`.
9. Delete `task.md`, `implementation_plan.md`, `walkthrough.md` from the active `/brain` directory since the work is completed and persisted offline, concluding the session gracefully.
10. Notify the user that the plan lifecycle is fully concluded, documentation generated, Git branch saved, and build environment released.
