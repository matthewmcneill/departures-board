---
description: Pull a plan's rich context from disk into your active session and mark it as WIP.
---

1. Identify the Target Plan ID provided by the user in `/plan-load [Target Plan ID]`.
2. Read the contexts from `.agents/plans/[Target Plan ID]/` including `PLAN.md`, `implementation_plan.md`, `task.md`, and `context_bridge.md`.
3. Append the *current* session ID to the `.agents/plans/[Target Plan ID]/sessions.md` lineage file.
4. Update the `.agents/plans/[Target Plan ID]/PLAN.md` YAML frontmatter to update `status: WIP`.
5. **DO NOT AUTO-EXECUTE OR CLAIM HARDWARE LOCKS.** 
7. **Proactive Review**:
   - Check if an `implementation_plan.md` exists in the loaded directory.
   - **If it exists**: Display the plan to the user and explicitly request review and approval to proceed with the execution stage (`/plan-start`).
   - **If NOT found**: Notify the user that no existing implementation plan was discovered and await instructions to draft a new one.
8. This ensures the agent is ready to resume momentum immediately upon reloading.
