# Explicit Execution Permission Rule

You are permitted to skip the formal `PLANNING` mode, `task.md`, and `implementation_plan.md` artifacts for purely trivial tasks (e.g., quick investigations, simple bug fixes, single-line edits). However, you are **STRICTLY PROHIBITED** from executing any code modifications autonomously without explicit permission.

### Workflow for Trivial Tasks:
1. **Investigate & Propose**: Once you investigate the codebase, find the issue, and determine the trivial fix, briefly explain what you found and precisely what you plan to change (the file and the logic).
2. **Stop and Ask**: You MUST stop immediately and ask for explicit permission to execute the edit. For example: *"I've found the issue and it requires a simple change in `appContext.cpp`. May I proceed with the fix?"*
3. **Wait for Approval**: Do NOT use any file editing tools (`replace_file_content`, `multi_replace_file_content`, `write_to_file`) or run commands that alter project state until the user explicitly responds with affirmative execution approval.

### Workflow for Complex Tasks:
- For multi-file or architectural tickets, you MUST utilize the 6-step `/plan-*` lifecycle workflows. 
- Draft the `implementation_plan.md`, test your hypotheses securely, and if paused mid-flight, safely persist context using the `/plan-save` workflow without modifying code prematurely.

### Scratch Files Workspace Constraint:
- When writing python scripts, data parsers, or temporary files to assist with your workflows (e.g. iterating over 40 plan files), you **MUST** write them to `.agents/plans/workspace/`. 
- You are **STRICTLY PROHIBITED** from using the global `/tmp/` directory, as this throws security prompts for the user and interrupts execution.

*Rationale*: This project frequently runs multiple concurrent agent sessions and background compilations. Unapproved, surprise file edits will cause severe build conflicts, lock violations, and corruption.
