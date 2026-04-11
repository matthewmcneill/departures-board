---
description: Concludes the plan lifecycle including house style checks, validation, git commits, final plan save, and releasing locks.
---

1. Determine the active Plan ID based on your session lineage (e.g., by checking if this session's ID exists inside any `.agents/plans/*/sessions.md` file).
2. **Refactoring & Documentation**: Run the `house-style-documentation` and `architectural-refactoring` skills to ensure all modified code and `/docs` files are strictly aligned with project standards.
3. **Validation & Assessment**: Execute targeted hardware compilation and validation per the guidelines inside `@[.agents/skills/pio-manager]`. Update `.agents/todo_list.md` with any discovered technical debt or necessary test suite additions.
4. **Git Commit & Metadata Update**: Prepare a "rich" Git commit summarizing the architectural decisions and breaking changes. Update the `.agents/plans/[ID]/PLAN.md` YAML with the final commit ID and set `status: DONE`. Ensure the plan's summary is fully captured in the `PLAN.md` body.
5. **Memory Compaction**: 
   - Remove the plan's entry from `.agents/plans/ACTIVE_PLANS.md`.
   - Append a one-liner entry to `.agents/plans/ARCHIVED_PLANS.md` (Format: `[Date] | [Title] | [Key Impact] | [Link]`).
   - Update the `Module Index` in `project_index.md` by moving the link from the module's `Active Plans` to its `Archive Highlights`.
6. Run an implicit `/plan-save` to ensure the final 100% completed `task.md` and `implementation_plan.md` and final clean states are safely persisted to `.agents/plans/[ID]/` (You MUST explicitly invoke the `distillery` skill and pass `.agents/plans/[ID]/` as the target directory to prevent artifacts from stranding in context_archives).
7. Finalize the operation by physically moving the completed plan's directory into the archive: `mv .agents/plans/[ID] .agents/plans/done/`
8. Safely free the hardware mutex protocols per the releasing guidelines in `@[.agents/skills/pio-manager]`.
9. Delete `task.md`, `implementation_plan.md`, `walkthrough.md` from the active `/brain` directory since the work is completed and persisted offline, concluding the session gracefully.
10. Notify the user that the plan lifecycle is fully concluded, documentation generated, Git branch saved, and build environment released.
