---
description: Queue a saved plan by formally marking it as READY for execution.
---

1. Attempt to resolve the current active Plan ID via `.agents/plans/*/sessions.md` lineage or via user argument `/plan-queue [ID]`.
2. Check if the `.agents/plans/[ID]/PLAN.md` file exists.
3. **If NO matching plan is found on disk**:
   - Immediately execute the `/plan-save` workflow natively to persist the `implementation_plan.md`, `task.md`, and context artifacts.
4. **Once the plan is persisted (or if it already existed)**:
   - Update the `.agents/plans/[ID]/PLAN.md` YAML frontmatter to explicitly set `status: READY`.
5. Notify the user confirming the queue placement and whether an implicit save was performed.
6. **CRITICAL HARD STOP**: You are **STRICTLY PROHIBITED** from beginning implementation, editing code, or executing the plan at this time. `/plan-queue` is a scheduling action ONLY. You MUST stop and yield control to the user. Do not proceed until explicitly requested via `/plan-start`.
