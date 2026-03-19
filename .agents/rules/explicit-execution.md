# Explicit Execution Permission Rule

You are permitted to skip the formal `PLANNING` mode, `task.md`, and `implementation_plan.md` artifacts for trivial tasks (e.g., quick investigations, simple bug fixes, single-file edits). However, you are **STRICTLY PROHIBITED** from executing any code modifications autonomously without explicit permission.

### Workflow for Trivial Tasks:
1. **Investigate & Propose**: Once you investigate the codebase, find the issue, and determine the trivial fix, briefly explain what you found and precisely what you plan to change (the file and the logic).
2. **Stop and Ask**: You MUST stop immediately and ask for explicit permission to execute the edit. For example: *"I've found the issue and it requires a simple change in `appContext.cpp`. May I proceed with the fix?"*
3. **Wait for Approval**: Do NOT use any file editing tools (`replace_file_content`, `multi_replace_file_content`, `write_to_file`) or run commands that alter project state until the user explicitly responds with affirmative execution approval.

*Rationale*: This project frequently runs multiple concurrent agent sessions and background compilations. Unapproved, surprise file edits will cause severe build conflicts, lock violations, and corruption.
