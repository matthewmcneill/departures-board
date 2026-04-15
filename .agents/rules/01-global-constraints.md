---
trigger: always_on
---

# 01 - Global Constraints

This rule applies to all agent activities in the project.

## Explicit Execution Permission
You are ONLY permitted to skip the formal `PLANNING` mode, `task.md`, and `implementation_plan.md` artifacts for purely trivial tasks (e.g., quick investigations, simple bug fixes, single-line edits) where a session has already previously approved of execution. However, you are **STRICTLY PROHIBITED** from executing any code modifications autonomously without explicit permission.

### Workflow:
1. **Investigate & Propose**: Once you find the issue and determine the trivial fix, briefly explain what you found and what you plan to change.
2. **Stop and Ask**: You MUST stop and ask for permission.
3. **Wait for Approval**: Do NOT use file editing tools until the user responds affirmatively.

### Scratch Files Constraint:
- When writing scripts or temporary files, you MUST write them to `.agents/tmp/`. You are **STRICTLY PROHIBITED** from using `/tmp/`.

## Implicit Workflow Prohibition
You are **STRICTLY PROHIBITED** from automatically executing administrative workflows (e.g., `/plan-wrap`, `/plan-save`, `/plan-start`) based on inference or conversational prompts like "Continue", "Looks good", or "Done". You MUST wait for the user to explicitly type the exact slash command before initiating any plan lifecycle or teardown procedures.

## Repository-Relative Portability
You are **STRICTLY PROHIBITED** from using absolute paths to the project repository (e.g., `/Users/myuser/Documents/...`) in ANY scripts, temporary files, plan documents (`implementation_plan.md`, `context_bridge.md`, `task.md`, `PLAN.md`), or terminal commands.
All authoring MUST use repository-relative paths to ensure portability across developer machines. 

*(Exception: The internal session workspace `<appDataDir>/brain/<conversation-id>/...` must use absolute paths)*

## Project Memory & Context Retrieval
Before you research, plan, or execute complex changes, you MUST read `.agents/project_index.md`. This file serves as the central project memory directory. It dictates the current state of the architecture, ongoing plans, module dependencies, and unresolved technical debt. Always interrogate it to acquire required context before drafting an `implementation_plan.md` or proposing system changes.