# Repository-Relative Portability Rule

To ensure that development plans, context bridges, and workflows remain portable across different developer machines and environments, you MUST adhere to the following path handling standards.

## Rule: No Absolute Workspace Paths

You are **STRICTLY PROHIBITED** from using absolute paths to the project repository (e.g., `/Users/mcneillm/Documents/Projects/departures-board/...`) in any of the following:
1.  **`implementation_plan.md`**: All file links MUST be relative to the repository root (e.g., `modules/foo/bar.cpp`).
2.  **`context_bridge.md`**: All references to codebase status or next steps MUST use relative paths.
3.  **`task.md`**: All task items referencing files MUST use relative paths.
4.  **`PLAN.md`**: Metadata and descriptions MUST remain workspace-agnostic.
5.  **Workflows**: All shell scripts and tool calls within workflows MUST use relative paths for in-repo resources.

## The "Brain" Exception

The only allowed absolute paths are those pointing to your internal session directory (the **"Brain"**), typically located at:
- `<appDataDir>/brain/<conversation-id>/...`

These absolute paths are required because the brain directory resides outside the workspace root and is unique to the current agent session.

## Verification Checklist
- [ ] Are all code references in the implementation plan relative?
- [ ] Did I strip the common repo prefix (e.g. `/Users/.../departures-board/`) from all links?
- [ ] Are shell commands in the walkthrough using relative paths?

*Rationale*: Hardcoded absolute paths break the ability for a second agent to load a saved plan if their local project path differs from the first agent's path.
